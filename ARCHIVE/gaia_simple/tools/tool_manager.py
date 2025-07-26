"""
Tool Manager Module

Responsible for:
- Managing available tools
- Routing tool requests
- Handling tool initialization and cleanup
- Providing unified interface for tool usage
"""

from typing import Dict, Any, List, Optional
from abc import ABC, abstractmethod


class Tool(ABC):
    """Base class for all tools."""
    
    @property
    @abstractmethod
    def name(self) -> str:
        """Tool name."""
        pass
    
    @property
    @abstractmethod
    def description(self) -> str:
        """Tool description."""
        pass
    
    @abstractmethod
    async def execute(self, **kwargs) -> Any:
        """Execute the tool with given parameters."""
        pass


class ToolManager:
    """Manages and coordinates tool usage."""
    
    def __init__(self):
        self.tools: Dict[str, Tool] = {}
        self._initialize_default_tools()
    
    def _initialize_default_tools(self):
        """Initialize the default set of tools."""
        # TODO: Register default tools
        pass
    
    def register_tool(self, tool: Tool):
        """Register a new tool."""
        self.tools[tool.name] = tool
    
    def get_tool(self, name: str) -> Optional[Tool]:
        """Get a tool by name."""
        return self.tools.get(name)
    
    async def execute_tool(self, tool_name: str, **kwargs) -> Any:
        """Execute a tool by name with given parameters."""
        tool = self.get_tool(tool_name)
        if not tool:
            raise ValueError(f"Tool '{tool_name}' not found")
        return await tool.execute(**kwargs)
    
    def list_tools(self) -> List[Dict[str, str]]:
        """List all available tools."""
        return [
            {"name": tool.name, "description": tool.description}
            for tool in self.tools.values()
        ]