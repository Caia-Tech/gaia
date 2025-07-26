import { GaiaCircuit } from './base.js';
export class SuperpositionGate extends GaiaCircuit {
    superpositionGates;
    collapseHistory;
    constructor(name, config = {}) {
        super(name, config);
        this.superpositionGates = new Map();
        this.collapseHistory = [];
        this.initializeSuperposition();
    }
    initializeSuperposition() {
        // Convert regular gates to superposition gates
        this.gates.forEach((gate, id) => {
            this.superpositionGates.set(id, {
                ...gate,
                states: [
                    { probability: 0.5, value: 0 },
                    { probability: 0.5, value: 1 },
                ],
                collapsed: false,
            });
        });
    }
    execute(inputs) {
        this.executionTrace = [`Executing ${this.name} in superposition with inputs: [${inputs}]`];
        this.collapseHistory = [];
        // Reset superposition states
        this.superpositionGates.forEach(gate => {
            gate.collapsed = false;
            gate.states = [
                { probability: 0.5, value: 0 },
                { probability: 0.5, value: 1 },
            ];
        });
        // Create input values map
        const values = new Map();
        inputs.forEach((val, idx) => {
            values.set(`IN${idx}`, val);
            this.executionTrace.push(`  IN${idx} = ${val}`);
        });
        // Execute with superposition
        const sorted = this.topologicalSort();
        for (const gateId of sorted) {
            const gate = this.superpositionGates.get(gateId);
            if (!gate)
                continue;
            // Calculate superposition states
            const inputValues = gate.inputs.map(input => values.get(input) || 0);
            this.updateSuperposition(gate, inputValues);
            // Collapse when needed
            if (this.shouldCollapse(gate)) {
                const collapsed = this.collapseGate(gate);
                values.set(gateId, collapsed);
                this.executionTrace.push(`  ${gateId} COLLAPSED → ${collapsed}`);
                this.collapseHistory.push(`${gateId}: ${this.getStateDescription(gate)}`);
            }
            else {
                // Use most probable state
                const probable = this.getMostProbableValue(gate);
                values.set(gateId, probable);
                this.executionTrace.push(`  ${gateId} SUPERPOSITION ${this.getStateDescription(gate)}`);
            }
        }
        // Collect outputs (force collapse for outputs)
        const outputs = [];
        for (let i = 0; i < this.outputCount; i++) {
            const outputGate = Array.from(this.connections.entries())
                .find(([_, conns]) => conns.includes(`OUT${i}`));
            if (outputGate) {
                const gate = this.superpositionGates.get(outputGate[0]);
                if (gate && !gate.collapsed) {
                    const collapsed = this.collapseGate(gate);
                    outputs.push(collapsed);
                    this.collapseHistory.push(`OUTPUT ${outputGate[0]}: ${collapsed}`);
                }
                else {
                    outputs.push(values.get(outputGate[0]) || 0);
                }
            }
            else {
                outputs.push(0);
            }
        }
        this.executionTrace.push(`  Outputs: [${outputs}]`);
        this.executionTrace.push(`\nCollapse History:\n  ${this.collapseHistory.join('\n  ')}`);
        return outputs;
    }
    updateSuperposition(gate, inputs) {
        // Calculate probability distribution based on gate type and inputs
        const certainty = this.calculateCertainty(gate, inputs);
        const expectedOutput = this.evaluateGate(gate, inputs);
        gate.states = [
            {
                probability: expectedOutput === 0 ? certainty : 1 - certainty,
                value: 0
            },
            {
                probability: expectedOutput === 1 ? certainty : 1 - certainty,
                value: 1
            },
        ];
    }
    calculateCertainty(gate, inputs) {
        // Higher certainty for more "definite" inputs
        switch (gate.type) {
            case 'AND':
                // Certain if all 1s or any 0
                return inputs.every(v => v === 1) || inputs.some(v => v === 0) ? 0.9 : 0.6;
            case 'OR':
                // Certain if any 1 or all 0s
                return inputs.some(v => v === 1) || inputs.every(v => v === 0) ? 0.9 : 0.6;
            case 'NOT':
                // Always certain for NOT
                return 0.95;
            case 'XOR':
                // Less certain for XOR
                return 0.7;
            default:
                return 0.75;
        }
    }
    shouldCollapse(gate) {
        // Collapse if:
        // 1. Already collapsed
        // 2. High certainty (> 0.8)
        // 3. Dependency requires it
        if (gate.collapsed)
            return false;
        const maxProb = Math.max(...gate.states.map(s => s.probability));
        return maxProb > 0.8;
    }
    collapseGate(gate) {
        // Collapse based on probability distribution
        const rand = Math.random();
        let cumulative = 0;
        for (const state of gate.states) {
            cumulative += state.probability;
            if (rand <= cumulative) {
                gate.collapsed = true;
                gate.output = state.value;
                return state.value;
            }
        }
        // Fallback
        gate.collapsed = true;
        gate.output = 0;
        return 0;
    }
    getMostProbableValue(gate) {
        let maxProb = 0;
        let value = 0;
        for (const state of gate.states) {
            if (state.probability > maxProb) {
                maxProb = state.probability;
                value = state.value;
            }
        }
        return value;
    }
    getStateDescription(gate) {
        return gate.states
            .map(s => `|${s.value}⟩:${(s.probability * 100).toFixed(0)}%`)
            .join(' + ');
    }
    observeStates() {
        const states = {};
        this.superpositionGates.forEach((gate, id) => {
            states[id] = {
                type: gate.type,
                collapsed: gate.collapsed,
                states: gate.states,
                description: this.getStateDescription(gate),
            };
        });
        return states;
    }
    getType() {
        return 'superposition';
    }
    visualize() {
        let viz = super.visualize();
        viz += '\nSuperposition States:\n';
        this.superpositionGates.forEach((gate, id) => {
            viz += `  ${id}: ${this.getStateDescription(gate)} ${gate.collapsed ? '[COLLAPSED]' : ''}\n`;
        });
        return viz;
    }
}
//# sourceMappingURL=superposition.js.map