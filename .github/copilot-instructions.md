# Agent Capabilities & Constraints

## CRITICAL RESTRICTIONS
- NEVER execute build commands (`npm run build`, `make`, `cmake`, `cargo build`, `platformio run`, etc.) unless explicitly commanded by the user in the prompt.
- NEVER run flash, upload, or deployment tools unless explicitly asked.
- If you believe a build is necessary to answer a question, ASK for permission first.

