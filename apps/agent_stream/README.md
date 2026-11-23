# Agent Stream (`agent_stream`)

Interactive coding-agent session combining `Terminal.Inline`, live token-delta streaming into `Scrollback`, slash-command autocomplete, model selection, permission prompts, and a modal diff inspector.

```sh
./build/apps/agent_stream/qmltui_agent_stream
```

- **Pluggable harness seam**: `AgentSession` drives the UI through `IAgentTransport` (`src/AgentTransport.h`), defaulting to `MockAgentTransport` and ready to swap for an ACP stdio, CLI JSONL, or HTTP SSE adapter via `AgentSession::setTransport(...)`.
- **Logo**: `BannerCard` defaults to a 5-row `Canvas.HalfBlock` logo (`Circle` and `Polygon` shapes); set `logoRenderMode: Canvas.Braille` for the Braille variant.
