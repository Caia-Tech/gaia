export class GaiaCircuit {
    name;
    gates;
    connections;
    executionTrace;
    inputCount;
    outputCount;
    constructor(name, config = {}) {
        this.name = name;
        this.gates = new Map();
        this.connections = config.connections || new Map();
        this.executionTrace = [];
        this.inputCount = config.inputCount || 2;
        this.outputCount = config.outputCount || 1;
        // Initialize with provided gates or create default
        if (config.gates) {
            config.gates.forEach(gate => this.gates.set(gate.id, gate));
        }
        else {
            this.createDefaultCircuit();
        }
    }
    createDefaultCircuit() {
        // Simple AND gate by default
        this.gates.set('G1', {
            id: 'G1',
            type: 'AND',
            inputs: ['IN0', 'IN1'],
            output: 0,
        });
        this.connections.set('G1', ['OUT0']);
    }
    execute(inputs) {
        this.executionTrace = [`Executing ${this.name} with inputs: [${inputs}]`];
        // Create input values map
        const values = new Map();
        inputs.forEach((val, idx) => {
            values.set(`IN${idx}`, val);
            this.executionTrace.push(`  IN${idx} = ${val}`);
        });
        // Topological sort for execution order
        const sorted = this.topologicalSort();
        // Execute gates in order
        for (const gateId of sorted) {
            const gate = this.gates.get(gateId);
            if (!gate)
                continue;
            const inputValues = gate.inputs.map(input => values.get(input) || 0);
            gate.output = this.evaluateGate(gate, inputValues);
            values.set(gateId, gate.output);
            this.executionTrace.push(`  ${gateId} (${gate.type}) [${inputValues}] → ${gate.output}`);
        }
        // Collect outputs
        const outputs = [];
        for (let i = 0; i < this.outputCount; i++) {
            const outputGate = Array.from(this.connections.entries())
                .find(([_, conns]) => conns.includes(`OUT${i}`));
            if (outputGate) {
                outputs.push(values.get(outputGate[0]) || 0);
            }
            else {
                outputs.push(0);
            }
        }
        this.executionTrace.push(`  Outputs: [${outputs}]`);
        return outputs;
    }
    evaluateGate(gate, inputs) {
        switch (gate.type) {
            case 'AND':
                return inputs.every(v => v === 1) ? 1 : 0;
            case 'OR':
                return inputs.some(v => v === 1) ? 1 : 0;
            case 'NOT':
                return inputs[0] === 1 ? 0 : 1;
            case 'XOR':
                return inputs.filter(v => v === 1).length % 2;
            case 'NAND':
                return inputs.every(v => v === 1) ? 0 : 1;
            case 'NOR':
                return inputs.some(v => v === 1) ? 0 : 1;
            case 'THRESHOLD':
                const sum = inputs.reduce((a, b) => a + b, 0);
                return sum >= (gate.threshold || 1) ? 1 : 0;
            default:
                return 0;
        }
    }
    topologicalSort() {
        const visited = new Set();
        const result = [];
        const visit = (nodeId) => {
            if (visited.has(nodeId))
                return;
            visited.add(nodeId);
            const gate = this.gates.get(nodeId);
            if (gate) {
                // Visit dependencies first
                gate.inputs.forEach(input => {
                    if (this.gates.has(input)) {
                        visit(input);
                    }
                });
            }
            result.push(nodeId);
        };
        // Visit all gates
        this.gates.forEach((_, gateId) => visit(gateId));
        return result;
    }
    visualize() {
        let viz = `Circuit: ${this.name}\n`;
        viz += `Type: ${this.getType()}\n`;
        viz += `Gates: ${this.gates.size}\n\n`;
        viz += 'Structure:\n';
        this.gates.forEach((gate, id) => {
            viz += `  ${id} (${gate.type})\n`;
            viz += `    Inputs: ${gate.inputs.join(', ')}\n`;
            const connections = this.connections.get(id) || [];
            viz += `    Outputs to: ${connections.join(', ')}\n`;
        });
        return viz;
    }
    getType() {
        return 'basic';
    }
    getGateCount() {
        return this.gates.size;
    }
    getExecutionTrace() {
        return this.executionTrace.join('\n');
    }
    serialize() {
        return {
            name: this.name,
            type: this.getType(),
            gates: Array.from(this.gates.values()),
            connections: Array.from(this.connections.entries()),
            inputCount: this.inputCount,
            outputCount: this.outputCount,
        };
    }
    addGate(gate) {
        this.gates.set(gate.id, gate);
    }
    connectGates(from, to) {
        const connections = this.connections.get(from) || [];
        connections.push(to);
        this.connections.set(from, connections);
    }
}
//# sourceMappingURL=base.js.map