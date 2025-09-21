package android.hardware.stats;

@VintfStability
interface IStats {
    parcelable CpuStats {
        int64 userTicks;
        int64 niceTicks;
        int64 systemTicks;
        int64 idleTicks;
        int64 iowaitTicks;
        int64 irqTicks;
        int64 softirqTicks;
    }

    parcelable GpuStats {
        int32 usagePercent;
        int64 frequencyHz;
    }

    parcelable MemoryStats {
        int64 totalKb;
        int64 freeKb;
        int64 availableKb;
    }

    parcelable ThermalStats {
        int32[] temperatures;
    }

    parcelable PowerStats {
        int64 batteryCapacity;
        int32 batteryPercent;
    }

    // Métodos de la interfaz
    CpuStats getCpuStats();
    GpuStats getGpuStats();
    MemoryStats getMemoryStats();
    ThermalStats getThermalStats();
    PowerStats getPowerStats();
}