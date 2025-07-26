"""
Calculator Tool

Provides mathematical calculation capabilities.
"""

import math
import numpy as np
from typing import Union, Any
from .tool_manager import Tool


class CalculatorTool(Tool):
    """Tool for performing mathematical calculations."""
    
    @property
    def name(self) -> str:
        return "calculator"
    
    @property
    def description(self) -> str:
        return "Perform mathematical calculations"
    
    def __init__(self):
        # Safe math functions
        self.safe_functions = {
            'sin': math.sin,
            'cos': math.cos,
            'tan': math.tan,
            'sqrt': math.sqrt,
            'log': math.log,
            'log10': math.log10,
            'exp': math.exp,
            'pow': pow,
            'abs': abs,
            'round': round,
            'floor': math.floor,
            'ceil': math.ceil,
            'pi': math.pi,
            'e': math.e
        }
    
    async def execute(self, expression: str) -> Union[float, int]:
        """
        Execute a mathematical calculation.
        
        Args:
            expression: Mathematical expression to evaluate
            
        Returns:
            Result of the calculation
        """
        try:
            # Create safe evaluation environment
            safe_dict = {
                '__builtins__': {},
                **self.safe_functions
            }
            
            # Evaluate expression safely
            result = eval(expression, safe_dict)
            return result
        except Exception as e:
            raise ValueError(f"Error evaluating expression: {str(e)}")
    
    def solve_equation(self, equation: str, variable: str = 'x') -> float:
        """Solve simple equations."""
        # TODO: Implement equation solving
        pass
    
    def statistical_analysis(self, data: list) -> dict:
        """Perform statistical analysis on data."""
        return {
            'mean': np.mean(data),
            'median': np.median(data),
            'std': np.std(data),
            'min': np.min(data),
            'max': np.max(data)
        }