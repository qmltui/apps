#include "HexBackend.h"

#include "BinaryDocument.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QtEndian>
#include <cstring>

namespace turbohex {

QStringList HexBackend::s_startupPaths;
bool HexBackend::s_forceDemo = false;
QHash<QString, QByteArray> HexBackend::s_demoStore;

namespace {

void putLe16(QByteArray &buf, int off, quint16 v) {
  const quint16 le = qToLittleEndian(v);
  std::memcpy(buf.data() + off, &le, 2);
}

void putLe32(QByteArray &buf, int off, quint32 v) {
  const quint32 le = qToLittleEndian(v);
  std::memcpy(buf.data() + off, &le, 4);
}

void putLe64(QByteArray &buf, int off, quint64 v) {
  const quint64 le = qToLittleEndian(v);
  std::memcpy(buf.data() + off, &le, 8);
}

void putBe32(QByteArray &buf, int off, quint32 v) {
  const quint32 be = qToBigEndian(v);
  std::memcpy(buf.data() + off, &be, 4);
}

QByteArray buildSampleElf64() {
  QByteArray elf(256, '\0');
  // ELF64 x86-64 Little-Endian header (64 bytes)
  elf[0] = '\x7F';
  elf[1] = 'E';
  elf[2] = 'L';
  elf[3] = 'F';
  elf[4] = 2;                 // ELFCLASS64
  elf[5] = 1;                 // ELFDATA2LSB
  elf[6] = 1;                 // EV_CURRENT
  elf[7] = 0;                 // ELFOSABI_SYSV
  putLe16(elf, 16, 2);        // ET_EXEC
  putLe16(elf, 18, 0x003E);   // EM_X86_64
  putLe32(elf, 20, 1);        // EV_CURRENT
  putLe64(elf, 24, 0x401000); // e_entry
  putLe64(elf, 32, 64);       // e_phoff
  putLe64(elf, 40, 192);      // e_shoff
  putLe16(elf, 52, 64);       // e_ehsize
  putLe16(elf, 54, 56);       // e_phentsize
  putLe16(elf, 56, 1);        // e_phnum
  putLe16(elf, 58, 64);       // e_shentsize
  putLe16(elf, 60, 1);        // e_shnum

  // Program Header #0: PT_LOAD (56 bytes at 0x40..0x77)
  putLe32(elf, 64, 1);          // PT_LOAD
  putLe32(elf, 68, 5);          // PF_R | PF_X
  putLe64(elf, 72, 0);          // p_offset
  putLe64(elf, 80, 0x00400000); // p_vaddr
  putLe64(elf, 88, 0x00400000); // p_paddr
  putLe64(elf, 96, 256);        // p_filesz
  putLe64(elf, 104, 256);       // p_memsz
  putLe64(elf, 112, 0x1000);    // p_align

  // .rodata strings & sample x86-64 epilogue bytes (0x78..0xBF)
  const QByteArray s1("QmlTui::TVision Firmware v1.4");
  elf.replace(0x78, s1.size(), s1);
  const QByteArray s2("ARCH=x86_64");
  elf.replace(0x98, s2.size(), s2);
  const QByteArray s3("STATUS=READY");
  elf.replace(0xA6, s3.size(), s3);
  // Sample machine instructions (`48 31 C0 C3` = xor rax,rax; ret)
  elf[0xB8] = '\x48';
  elf[0xB9] = '\x31';
  elf[0xBA] = '\xC0';
  elf[0xBB] = '\xC3';

  // Section Header #0 at 0xC0..0xFF
  putLe32(elf, 0xC4, 1); // SHT_PROGBITS
  putLe64(elf, 0xD0, 0x401000);
  putLe64(elf, 0xD8, 0x78);
  putLe64(elf, 0xE0, 0x48);
  return elf;
}

QByteArray buildSamplePng() {
  QByteArray png(96, '\0');
  const char sig[8] = {'\x89', 'P', 'N', 'G', '\r', '\n', '\x1A', '\n'};
  std::memcpy(png.data(), sig, 8);

  // IHDR chunk at 0x08 (13 bytes payload -> 25 bytes total: 0x08..0x20)
  putBe32(png, 8, 13);
  std::memcpy(png.data() + 12, "IHDR", 4);
  putBe32(png, 16, 32); // width = 32
  putBe32(png, 20, 16); // height = 16
  png[24] = 8;          // bit depth
  png[25] = 6;          // RGBA
  putBe32(png, 29, 0x907753DE);

  // tEXt chunk at 0x21 (25 bytes payload -> 37 bytes total: 0x21..0x45)
  const QByteArray txtPayload("Software\0TurboHex QmlTui", 24);
  putBe32(png, 33, static_cast<quint32>(txtPayload.size()));
  std::memcpy(png.data() + 37, "tEXt", 4);
  png.replace(41, txtPayload.size(), txtPayload);
  putBe32(png, 65, 0x3A7B11C2);

  // IDAT chunk at 0x45 (6 bytes payload -> 18 bytes total: 0x45..0x56)
  putBe32(png, 69, 3);
  std::memcpy(png.data() + 73, "IDAT", 4);
  png[77] = '\x78';
  png[78] = '\x9C';
  png[79] = '\x03';
  putBe32(png, 80, 0x00010001);

  // IEND chunk at 0x54..0x5F (12 bytes)
  putBe32(png, 84, 0);
  std::memcpy(png.data() + 88, "IEND", 4);
  putBe32(png, 92, 0xAE426082);
  return png;
}

} // namespace

void HexBackend::ensureDemoStoreSeeded() {
  if (!s_demoStore.isEmpty())
    return;
  s_demoStore.insert(QStringLiteral("demo://firmware.elf"), buildSampleElf64());
  s_demoStore.insert(QStringLiteral("demo://badge.png"), buildSamplePng());
}

QByteArray HexBackend::demoFileBytes(const QString &uri) {
  ensureDemoStoreSeeded();
  return s_demoStore.value(uri);
}

void HexBackend::updateDemoFileBytes(const QString &uri,
                                     const QByteArray &bytes) {
  ensureDemoStoreSeeded();
  s_demoStore.insert(uri, bytes);
}

void HexBackend::configureStartup(const QStringList &positionalPaths,
                                  bool forceDemo) {
  s_startupPaths = positionalPaths;
  s_forceDemo = forceDemo;
}

HexBackend *HexBackend::create(QQmlEngine *qmlEngine, QJSEngine *jsEngine) {
  Q_UNUSED(qmlEngine);
  Q_UNUSED(jsEngine);
  return new HexBackend();
}

HexBackend::HexBackend(QObject *parent) : QObject(parent) {
  ensureDemoStoreSeeded();
  m_demoMode = s_forceDemo;

  if (m_demoMode) {
    m_rootPath = QStringLiteral("demo://");
    m_initialFiles = {QStringLiteral("demo://firmware.elf")};
    return;
  }

  m_rootPath = QDir::currentPath();
  for (const QString &arg : std::as_const(s_startupPaths)) {
    const QFileInfo info(arg);
    if (info.exists() && info.isDir()) {
      m_rootPath = info.absoluteFilePath();
    } else if (info.exists() && info.isFile()) {
      m_initialFiles.append(info.absoluteFilePath());
      if (m_initialFiles.size() == 1)
        m_rootPath = info.absolutePath();
    }
  }

  if (m_initialFiles.isEmpty()) {
    // Still let users launch `./qmltui_turbo_hex` with zero args on desktop:
    // open demo firmware.elf initially while keeping real filesystem F3 Open!
    m_initialFiles = {QStringLiteral("demo://firmware.elf")};
  }
}

void HexBackend::setRootPath(const QString &path) {
  if (path.trimmed().isEmpty() || m_rootPath == path)
    return;
  m_rootPath = path;
  emit rootPathChanged();
}

QString HexBackend::resolvePath(const QString &baseDir,
                                const QString &inputPath) const {
  const QString trimmed = inputPath.trimmed();
  if (trimmed.isEmpty() || trimmed.startsWith(QStringLiteral("demo://")))
    return trimmed;
  if (m_demoMode)
    return QStringLiteral("demo://") + QFileInfo(trimmed).fileName();
  const QFileInfo info(trimmed);
  if (info.isAbsolute())
    return QDir::cleanPath(trimmed);
  const QString base = baseDir.isEmpty() ? m_rootPath : baseDir;
  return QDir::cleanPath(QDir(base).filePath(trimmed));
}

QVariantList HexBackend::listDirectory(const QString &dirPath,
                                       const QString &mask) const {
  const QString trimmedMask = mask.trimmed();
  const bool filterActive = !trimmedMask.isEmpty() &&
                            trimmedMask != QStringLiteral("*") &&
                            trimmedMask != QStringLiteral("*.*");
  QRegularExpression maskRe;
  if (filterActive) {
    const QString pat = QRegularExpression::wildcardToRegularExpression(
        trimmedMask, QRegularExpression::UnanchoredWildcardConversion);
    maskRe = QRegularExpression(QStringLiteral("^") + pat + QStringLiteral("$"),
                                QRegularExpression::CaseInsensitiveOption);
  }

  QVariantList out;
  auto appendDemoFiles = [&]() {
    for (auto it = s_demoStore.cbegin(); it != s_demoStore.cend(); ++it) {
      const QString name = it.key().mid(7);
      if (filterActive && !maskRe.match(name).hasMatch())
        continue;
      QVariantMap row;
      row.insert(QStringLiteral("name"), name);
      row.insert(QStringLiteral("path"), it.key());
      row.insert(QStringLiteral("isDir"), false);
      row.insert(QStringLiteral("format"),
                 BinaryDocument::detectFormatBadge(it.value()));
      row.insert(QStringLiteral("bytes"), it.value().size());
      out.append(row);
    }
  };

  if (m_demoMode || dirPath == QStringLiteral("demo://")) {
    appendDemoFiles();
    return out;
  }

  const QString targetDir =
      dirPath.trimmed().isEmpty() ? m_rootPath : QDir::cleanPath(dirPath);
  const QDir dir(targetDir);
  if (!dir.exists()) {
    appendDemoFiles();
    return out;
  }

  QDir parentDir(dir);
  if (parentDir.cdUp()) {
    QVariantMap upRow;
    upRow.insert(QStringLiteral("name"), QStringLiteral(".."));
    upRow.insert(QStringLiteral("path"), parentDir.absolutePath());
    upRow.insert(QStringLiteral("isDir"), true);
    upRow.insert(QStringLiteral("format"), QStringLiteral("DIR"));
    upRow.insert(QStringLiteral("bytes"), 0);
    out.append(upRow);
  }

  const QFileInfoList entries =
      dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot,
                        QDir::DirsFirst | QDir::Name | QDir::IgnoreCase);
  for (const QFileInfo &info : entries) {
    const QString name = info.fileName();
    if (name.startsWith(QLatin1Char('.')))
      continue;
    if (info.isDir()) {
      QVariantMap row;
      row.insert(QStringLiteral("name"), name + QLatin1Char('/'));
      row.insert(QStringLiteral("path"), info.absoluteFilePath());
      row.insert(QStringLiteral("isDir"), true);
      row.insert(QStringLiteral("format"), QStringLiteral("DIR"));
      row.insert(QStringLiteral("bytes"), 0);
      out.append(row);
    } else {
      if (filterActive && !maskRe.match(name).hasMatch())
        continue;
      QByteArray head;
      QFile probe(info.absoluteFilePath());
      if (probe.open(QIODevice::ReadOnly))
        head = probe.read(16);
      QVariantMap row;
      row.insert(QStringLiteral("name"), name);
      row.insert(QStringLiteral("path"), info.absoluteFilePath());
      row.insert(QStringLiteral("isDir"), false);
      row.insert(QStringLiteral("format"),
                 BinaryDocument::detectFormatBadge(head));
      row.insert(QStringLiteral("bytes"), info.size());
      out.append(row);
    }
  }
  return out;
}

} // namespace turbohex
