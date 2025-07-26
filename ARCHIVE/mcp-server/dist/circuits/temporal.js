import { GaiaCircuit } from './base.js';
export class TemporalCircuit extends GaiaCircuit {
    temporalGates;
    timeStep;
    constructor(name, config = {}) {
        super(name, config);
        this.temporalGates = new Map();
        this.timeStep = 0;
        this.initializeTemporal(config.memorySize || 5);
    }
    initializeTemporal(defaultMemorySize) {
        // Convert regular gates to temporal gates
        this.gates.forEach((gate, id) => {
            this.temporalGates.set(id, {
                ...gate,
                history: [],
                memorySize: defaultMemorySize,
                temporalFunction: this.assignTemporalFunction(gate.type),
            });
        });
    }
    assignTemporalFunction(gateType) {
        // Assign temporal behaviors based on gate type
        switch (gateType) {
            case 'AND':
            case 'OR':
                return 'INTEGRATE';
            case 'NOT':
                return 'DELAY';
            case 'XOR':
                return 'DIFFERENTIATE';
            default:
                return 'PATTERN';
        }
    }
    execute(inputs) {
        this.timeStep++;
        this.executionTrace = [`Executing ${this.name} at time ${this.timeStep} with inputs: [${inputs}]`];
        // Create input values map
        const values = new Map();
        inputs.forEach((val, idx) => {
            values.set(`IN${idx}`, val);
            this.executionTrace.push(`  IN${idx} = ${val}`);
        });
        // Execute gates with temporal logic
        const sorted = this.topologicalSort();
        for (const gateId of sorted) {
            const gate = this.temporalGates.get(gateId);
            if (!gate)
                continue;
            const inputValues = gate.inputs.map(input => values.get(input) || 0);
            const baseOutput = this.evaluateGate(gate, inputValues);
            // Apply temporal function
            const temporalOutput = this.applyTemporalFunction(gate, baseOutput);
            gate.output = temporalOutput;
            values.set(gateId, temporalOutput);
            // Update history
            gate.history.push(temporalOutput);
            if (gate.history.length > gate.memorySize) {
                gate.history.shift();
            }
            this.executionTrace.push(`  ${gateId} (${gate.type}/${gate.temporalFunction}) [${inputValues}] → ${baseOutput} → ${temporalOutput} | History: [${gate.history}]`);
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
    applyTemporalFunction(gate, currentValue) {
        if (gate.history.length === 0)
            return currentValue;
        switch (gate.temporalFunction) {
            case 'DELAY':
                // Output previous value
                return gate.history[gate.history.length - 1];
            case 'INTEGRATE':
                // Output 1 if sum of history > threshold
                const sum = gate.history.reduce((a, b) => a + b, 0) + currentValue;
                return sum >= Math.ceil(gate.memorySize / 2) ? 1 : 0;
            case 'DIFFERENTIATE':
                // Output 1 if value changed
                const prev = gate.history[gate.history.length - 1];
                return prev !== currentValue ? 1 : 0;
            case 'PATTERN':
                // Detect repeating patterns
                if (gate.history.length >= 3) {
                    const pattern = this.detectPattern(gate.history);
                    if (pattern) {
                        // Predict next based on pattern
                        return pattern.next;
                    }
                }
                return currentValue;
            default:
                return currentValue;
        }
    }
    detectPattern(history) {
        // Simple pattern detection - look for repeating sequences
        for (let len = 2; len <= Math.floor(history.length / 2); len++) {
            let isPattern = true;
            for (let i = 0; i < len; i++) {
                if (history[history.length - len + i] !== history[history.length - 2 * len + i]) {
                    isPattern = false;
                    break;
                }
            }
            if (isPattern) {
                // Pattern found, predict next
                const position = history.length % len;
                const next = history[history.length - len + position];
                return { length: len, next };
            }
        }
        return null;
    }
    getMemoryState() {
        const memory = {};
        this.temporalGates.forEach((gate, id) => {
            memory[id] = {
                type: gate.type,
                temporalFunction: gate.temporalFunction,
                history: gate.history,
                memorySize: gate.memorySize,
                currentOutput: gate.output,
            };
        });
        return {
            timeStep: this.timeStep,
            gates: memory,
        };
    }
    resetMemory() {
        this.timeStep = 0;
        this.temporalGates.forEach(gate => {
            gate.history = [];
        });
    }
    getType() {
        return 'temporal';
    }
    visualize() {
        let viz = super.visualize();
        viz += `\nTime Step: ${this.timeStep}\n`;
        viz += '\nTemporal States:\n';
        this.temporalGates.forEach((gate, id) => {
            viz += `  ${id} (${gate.temporalFunction}): History=[${gate.history}]\n`;
        });
        return viz;
    }
}
//# sourceMappingURL=temporal.js.map