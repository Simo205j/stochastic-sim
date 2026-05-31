#pragma once

#include <cstddef>
#include <fstream>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace stochastic::examples
{

    // R6: Helper used by the example programs to write simulation trajectories
    // as CSV files for external plotting/visualisation.
    class TrajectoryWriter
    {
    public:
        // R6: Opens the output CSV file and writes the trajectory column header.
        explicit TrajectoryWriter(
            std::string output_path,
            std::initializer_list<std::string_view> column_names)
            : _output_path{std::move(output_path)}, _out{_output_path}
        {
            if (!_out)
            {
                throw std::runtime_error{
                    "Failed to open trajectory output file: " + _output_path};
            }

            write_header(column_names);
        }

        // R6: File streams are owned resources, so copying the writer is disabled.
        TrajectoryWriter(const TrajectoryWriter &) = delete;
        TrajectoryWriter &operator=(const TrajectoryWriter &) = delete;

        // R6: Moving is allowed so ownership of the output stream can be transferred.
        TrajectoryWriter(TrajectoryWriter &&) noexcept = default;
        TrajectoryWriter &operator=(TrajectoryWriter &&) noexcept = default;

        // R6: Writes one observed trajectory state using inline amount values.
        void write_row(double time, std::initializer_list<std::size_t> amounts)
        {
            _out << time;

            for (const auto amount : amounts)
            {
                _out << ',' << amount;
            }

            _out << '\n';

            if (!_out)
            {
                throw std::runtime_error{
                    "Failed while writing trajectory row to: " + _output_path};
            }
        }

        // R6: Writes one observed trajectory state from a state/amount vector.
        void write_row(double time, const std::vector<std::size_t> &amounts)
        {
            _out << time;

            for (const auto amount : amounts)
            {
                _out << ',' << amount;
            }

            _out << '\n';

            if (!_out)
            {
                throw std::runtime_error{
                    "Failed while writing trajectory row to: " + _output_path};
            }
        }

    private:
        // R6: Writes the CSV header used by the plotting scripts.
        void write_header(std::initializer_list<std::string_view> column_names)
        {
            auto first = true;

            for (const auto column_name : column_names)
            {
                if (!first)
                {
                    _out << ',';
                }

                _out << column_name;
                first = false;
            }

            _out << '\n';

            if (!_out)
            {
                throw std::runtime_error{
                    "Failed while writing trajectory header to: " + _output_path};
            }
        }

        // R6: Stores the path for error messages and owns the CSV output stream.
        std::string _output_path;
        std::ofstream _out;
    };

} // namespace stochastic::examples