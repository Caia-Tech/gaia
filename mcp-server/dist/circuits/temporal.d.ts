import { GaiaCircuit, CircuitConfig } from './base.js';
export declare class TemporalCircuit extends GaiaCircuit {
    private temporalGates;
    private timeStep;
    constructor(name: string, config?: CircuitConfig & {
        memorySize?: number;
    });
    private initializeTemporal;
    private assignTemporalFunction;
    execute(inputs: number[]): number[];
    private applyTemporalFunction;
    private detectPattern;
    getMemoryState(): any;
    resetMemory(): void;
    getType(): string;
    visualize(): string;
}
//# sourceMappingURL=temporal.d.ts.map