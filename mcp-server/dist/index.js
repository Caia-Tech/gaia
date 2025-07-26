#!/usr/bin/env node
import { Server } from '@modelcontextprotocol/sdk/server/index.js';
import { StdioServerTransport } from '@modelcontextprotocol/sdk/server/stdio.js';
import { CallToolRequestSchema, ListResourcesRequestSchema, ListToolsRequestSchema, ReadResourceRequestSchema, } from '@modelcontextprotocol/sdk/types.js';
import { GaiaCircuit } from './circuits/base.js';
import { SuperpositionGate } from './circuits/superposition.js';
import { TemporalCircuit } from './circuits/temporal.js';
import { AdaptiveTopology } from './circuits/adaptive.js';
// Circuit registry
const circuits = new Map();
// Create MCP server
const server = new Server({
    name: 'gaia-mcp-server',
    version: '1.0.0',
}, {
    capabilities: {
        resources: {},
        tools: {},
    },
});
// Handle tool listing
server.setRequestHandler(ListToolsRequestSchema, async () => {
    return {
        tools: [
            {
                name: 'create_circuit',
                description: 'Create a new GAIA logic circuit',
                inputSchema: {
                    type: 'object',
                    properties: {
                        name: {
                            type: 'string',
                            description: 'Circuit name',
                        },
                        type: {
                            type: 'string',
                            enum: ['basic', 'superposition', 'temporal', 'adaptive'],
                            description: 'Circuit architecture type',
                        },
                        config: {
                            type: 'object',
                            description: 'Circuit configuration',
                        },
                    },
                    required: ['name', 'type'],
                },
            },
            {
                name: 'execute_circuit',
                description: 'Execute a circuit with given inputs',
                inputSchema: {
                    type: 'object',
                    properties: {
                        name: {
                            type: 'string',
                            description: 'Circuit name',
                        },
                        inputs: {
                            type: 'array',
                            items: { type: 'number' },
                            description: 'Input values (0 or 1)',
                        },
                    },
                    required: ['name', 'inputs'],
                },
            },
            {
                name: 'visualize_circuit',
                description: 'Get circuit visualization data',
                inputSchema: {
                    type: 'object',
                    properties: {
                        name: {
                            type: 'string',
                            description: 'Circuit name',
                        },
                    },
                    required: ['name'],
                },
            },
            {
                name: 'observe_superposition',
                description: 'Observe superposition states in a circuit',
                inputSchema: {
                    type: 'object',
                    properties: {
                        name: {
                            type: 'string',
                            description: 'Circuit name',
                        },
                    },
                    required: ['name'],
                },
            },
            {
                name: 'evolve_circuit',
                description: 'Evolve circuit topology based on performance',
                inputSchema: {
                    type: 'object',
                    properties: {
                        name: {
                            type: 'string',
                            description: 'Circuit name',
                        },
                        fitness_function: {
                            type: 'string',
                            description: 'JavaScript function to evaluate fitness',
                        },
                        iterations: {
                            type: 'number',
                            description: 'Evolution iterations',
                        },
                    },
                    required: ['name', 'fitness_function'],
                },
            },
        ],
    };
});
// Handle tool execution
server.setRequestHandler(CallToolRequestSchema, async (request) => {
    const { name, arguments: args } = request.params;
    switch (name) {
        case 'create_circuit': {
            const { name: circuitName, type, config = {} } = args;
            let circuit;
            switch (type) {
                case 'superposition':
                    circuit = new SuperpositionGate(circuitName, config);
                    break;
                case 'temporal':
                    circuit = new TemporalCircuit(circuitName, config);
                    break;
                case 'adaptive':
                    circuit = new AdaptiveTopology(circuitName, config);
                    break;
                default:
                    circuit = new GaiaCircuit(circuitName, config);
            }
            circuits.set(circuitName, circuit);
            return {
                content: [
                    {
                        type: 'text',
                        text: `Created ${type} circuit: ${circuitName}\nConfiguration: ${JSON.stringify(config, null, 2)}`,
                    },
                ],
            };
        }
        case 'execute_circuit': {
            const { name: circuitName, inputs } = args;
            const circuit = circuits.get(circuitName);
            if (!circuit) {
                throw new Error(`Circuit ${circuitName} not found`);
            }
            const result = circuit.execute(inputs);
            const trace = circuit.getExecutionTrace();
            return {
                content: [
                    {
                        type: 'text',
                        text: `Execution result: ${JSON.stringify(result)}\n\nTrace:\n${trace}`,
                    },
                ],
            };
        }
        case 'visualize_circuit': {
            const { name: circuitName } = args;
            const circuit = circuits.get(circuitName);
            if (!circuit) {
                throw new Error(`Circuit ${circuitName} not found`);
            }
            const visualization = circuit.visualize();
            return {
                content: [
                    {
                        type: 'text',
                        text: visualization,
                    },
                ],
            };
        }
        case 'observe_superposition': {
            const { name: circuitName } = args;
            const circuit = circuits.get(circuitName);
            if (!circuit || !(circuit instanceof SuperpositionGate)) {
                throw new Error(`Superposition circuit ${circuitName} not found`);
            }
            const states = circuit.observeStates();
            return {
                content: [
                    {
                        type: 'text',
                        text: `Superposition states:\n${JSON.stringify(states, null, 2)}`,
                    },
                ],
            };
        }
        case 'evolve_circuit': {
            const { name: circuitName, fitness_function, iterations = 100 } = args;
            const circuit = circuits.get(circuitName);
            if (!circuit || !(circuit instanceof AdaptiveTopology)) {
                throw new Error(`Adaptive circuit ${circuitName} not found`);
            }
            const evolutionLog = circuit.evolve(fitness_function, iterations);
            return {
                content: [
                    {
                        type: 'text',
                        text: `Evolution complete:\n${evolutionLog}`,
                    },
                ],
            };
        }
        default:
            throw new Error(`Unknown tool: ${name}`);
    }
});
// Handle resource listing
server.setRequestHandler(ListResourcesRequestSchema, async () => {
    const resources = Array.from(circuits.entries()).map(([name, circuit]) => ({
        uri: `circuit://${name}`,
        name: `Circuit: ${name}`,
        description: `${circuit.getType()} circuit with ${circuit.getGateCount()} gates`,
        mimeType: 'application/json',
    }));
    return { resources };
});
// Handle resource reading
server.setRequestHandler(ReadResourceRequestSchema, async (request) => {
    const { uri } = request.params;
    const match = uri.match(/^circuit:\/\/(.+)$/);
    if (!match) {
        throw new Error('Invalid resource URI');
    }
    const circuitName = match[1];
    const circuit = circuits.get(circuitName);
    if (!circuit) {
        throw new Error(`Circuit ${circuitName} not found`);
    }
    return {
        contents: [
            {
                uri,
                mimeType: 'application/json',
                text: JSON.stringify(circuit.serialize(), null, 2),
            },
        ],
    };
});
// Start the server
async function main() {
    const transport = new StdioServerTransport();
    await server.connect(transport);
    console.error('GAIA MCP Server running...');
}
main().catch((error) => {
    console.error('Server error:', error);
    process.exit(1);
});
//# sourceMappingURL=index.js.map