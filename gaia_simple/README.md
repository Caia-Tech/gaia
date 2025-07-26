# GAIA Simple

A simplified implementation of a GAIA benchmark solver focusing on core functionality.

## Project Structure

```
gaia_simple/
├── core/                    # Core modules
│   ├── question_analyzer.py # Analyzes questions and extracts requirements
│   ├── task_planner.py     # Creates step-by-step execution plans
│   ├── executor.py         # Executes tasks using tools
│   └── answer_generator.py # Generates final answers
├── tools/                  # Tool implementations
│   ├── tool_manager.py     # Manages and coordinates tools
│   ├── web_search.py       # Web search capabilities
│   ├── code_executor.py    # Safe code execution
│   ├── file_handler.py     # File reading and processing
│   └── calculator.py       # Mathematical calculations
├── utils/                  # Utility modules
│   ├── config.py          # Configuration management
│   └── logger.py          # Logging utilities
├── tests/                  # Test modules
└── main.py                # Main entry point
```

## Key Features

1. **Question Understanding**: Analyzes GAIA questions to identify requirements and determine approach
2. **Task Planning**: Creates structured plans to solve complex multi-step problems
3. **Tool Integration**: Integrates multiple tools for web search, calculations, code execution, and file handling
4. **Safe Execution**: Executes plans with error handling and timeout management
5. **Answer Generation**: Synthesizes results into properly formatted answers

## Usage

### Command Line
```bash
# Solve a question directly
python main.py "What is the capital of France?"

# Solve from file
python main.py --file question.txt

# With attachments
python main.py --file question.txt --attachments data.csv image.png
```

### Python API
```python
from gaia_simple.main import GAIASolver

solver = GAIASolver()
result = solver.solve_sync("What is 2+2?")
print(result['answer'])
```

## Configuration

Configuration can be customized via `~/.gaia_simple/config.json` or by passing a config file path.

## Development Status

This is a placeholder implementation. Core functionality needs to be implemented in each module.