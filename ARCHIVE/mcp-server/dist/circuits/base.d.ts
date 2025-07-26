export interface Gate {
    id: string;
    type: 'AND' | 'OR' | 'NOT' | 'XOR' | 'NAND' | 'NOR' | 'THRESHOLD';
    inputs: string[];
    output: number;
    threshold?: number;
}
export interface CircuitConfig {
    gates?: Gate[];
    connections?: Map<string, string[]>;
    inputCount?: number;
    outputCount?: number;
}
export declare class GaiaCircuit {
    protected name: string;
    protected gates: Map<string, Gate>;
    protected connections: Map<string, string[]>;
    protected executionTrace: string[];
    protected inputCount: number;
    protected outputCount: number;
    constructor(name: string, config?: CircuitConfig);
    protected createDefaultCircuit(): void;
    execute(inputs: number[]): number[];
    protected evaluateGate(gate: Gate, inputs: number[]): number;
    protected topologicalSort(): string[];
    visualize(): string;
    getType(): string;
    getGateCount(): number;
    getExecutionTrace(): string;
    serialize(): any;
    addGate(gate: Gate): void;
    connectGates(from: string, to: string): void;
}
//# sourceMappingURL=base.d.ts.map