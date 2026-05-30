from __future__ import annotations

from pathlib import Path

from plot_trajectory import plot_trajectory


ROOT = Path(__file__).resolve().parents[1]
PLOTS_DIR = ROOT / "plots"

CSV_FILES = [
    ROOT / "abc_decay_A50_B50_C1.csv",
    ROOT / "abc_decay_A100_B0_C1.csv",
    ROOT / "abc_decay_A100_B0_C2.csv",
    ROOT / "circadian_rhythm.csv",
    ROOT / "covid19_N10000.csv",
]


def main() -> None:
    PLOTS_DIR.mkdir(exist_ok=True)

    for csv_path in CSV_FILES:
        if not csv_path.exists():
            print(f"Skipping missing file: {csv_path}")
            continue

        output_path = PLOTS_DIR / f"{csv_path.stem}.png"
        plot_trajectory(csv_path, output_path)


if __name__ == "__main__":
    main()