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

    class TrajectoryWriter
    {
    public:
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

        TrajectoryWriter(const TrajectoryWriter &) = delete;
        TrajectoryWriter &operator=(const TrajectoryWriter &) = delete;

        TrajectoryWriter(TrajectoryWriter &&) noexcept = default;
        TrajectoryWriter &operator=(TrajectoryWriter &&) noexcept = default;

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

        std::string _output_path;
        std::ofstream _out;
    };

} // namespace stochastic::examples