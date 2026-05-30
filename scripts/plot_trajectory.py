from __future__ import annotations

import argparse
from pathlib import Path

import matplotlib.pyplot as plt
import pandas as pd


def plot_trajectory(csv_path: Path, output_path: Path | None = None) -> None:
    if not csv_path.exists():
        raise FileNotFoundError(f"CSV file does not exist: {csv_path}")

    df = pd.read_csv(csv_path)

    if df.empty:
        raise ValueError(f"CSV file is empty: {csv_path}")

    if len(df.columns) < 2:
        raise ValueError(
            f"Expected at least two columns in {csv_path}: time plus one species."
        )

    time_column = df.columns[0]
    species_columns = list(df.columns[1:])

    output_path = output_path or csv_path.with_suffix(".png")
    output_path.parent.mkdir(parents=True, exist_ok=True)

    plt.figure(figsize=(10, 6))

    for species in species_columns:
        plt.plot(df[time_column], df[species], label=species)

    title = csv_path.stem.replace("_", " ")
    plt.title(title)
    plt.xlabel(time_column)
    plt.ylabel("amount")
    plt.legend()
    plt.grid(True, alpha=0.3)
    plt.tight_layout()

    plt.savefig(output_path, dpi=200)
    plt.close()

    print(f"Wrote plot: {output_path}")


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Plot stochastic simulation trajectory CSV files."
    )
    parser.add_argument(
        "csv",
        type=Path,
        help="Path to the trajectory CSV file.",
    )
    parser.add_argument(
        "output",
        type=Path,
        nargs="?",
        default=None,
        help="Optional output PNG path. Defaults to the CSV name with .png.",
    )

    args = parser.parse_args()
    plot_trajectory(args.csv, args.output)


if __name__ == "__main__":
    main()