# Adoption roadmap worksheet (Session 5)

Codebase: ______________________   Current standard flag: ______   Target: ______

## Tier 1: mechanical, zero risk (this month)

| Change | Where (files/modules) | Owner | Done |
|---|---|---|---|
| make_unique / make_shared for every new | | | |
| [[nodiscard]] on value-returning functions | | | |
| Structured bindings for pair/tuple returns | | | |
| string_view at read-only boundaries | | | |
| optional for sentinel returns | | | |
| Run clang-tidy modernize-* and triage | | | |

## Tier 2: interface changes (this quarter)

| Change | Where | Owner | Done |
|---|---|---|---|
| span for buffer parameters | | | |
| expected for error-returning functions | | | |
| Concepts on public templates | | | |
| std::format / print replacing printf/iostream | | | |

## Tier 3: architectural (evaluate)

| Change | Candidate area | Decision | Notes |
|---|---|---|---|
| Ranges pipelines | | | |
| jthread and stop tokens | | | |
| Coroutines (generator, async) | | | |
| Modules | | | |
