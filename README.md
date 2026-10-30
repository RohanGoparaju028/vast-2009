# VAST 2009 — C++ / VTK Explorer

A desktop visual analytics project for the two supplied VAST Challenge 2009 datasets. All data loading, aggregation, reporting, and visualization code is C++17. VTK renders the interactive application; no Python runtime is required.

## Build and run

Requires a C++17 compiler, CMake 3.20+, and VTK 9 with OpenGL rendering and JPEG/PNG support. On macOS, install VTK with Homebrew in a normal terminal:

```sh
brew install vtk
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$(brew --prefix vtk)"
cmake --build build --parallel
./build/vast-explorer --data .
```

With VTK already installed on Linux or Windows, configure using its installation prefix or `-DVTK_DIR=/path/to/lib/cmake/vtk-9.x`. On Windows with a multi-configuration generator, build with `--config Release` and run `build/Release/vast-explorer.exe`.

The CMake setup uses explicit VTK modules and `vtk_module_autoinit`, following the [official VTK C++ build guide](https://docs.vtk.org/en/latest/getting_started/using_cpp.html).

### Analysis without a graphics dependency

This builds the same analysis engine and exports its results without requiring VTK or a display:

```sh
cmake -S . -B build-core -DVAST_BUILD_GUI=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build build-core --parallel
ctest --test-dir build-core --output-on-failure
./build-core/vast-analyze --data . --output reports
```

Run commands from this project directory, or pass its absolute path to `--data`. Both dataset subdirectories must remain underneath that path. Neither executable edits the input data. Report exports overwrite the six generated report files in the specified output folder.

## Interactive views

| Key | View | Interaction / meaning |
|---|---|---|
| `1` | Social network | Directed ego network around the selected person. Click a node to refocus; `F` switches to all 6,000 people grouped schematically by city. |
| `2` | Employee comparison | Every employee, ranked by after-hours classified entries and badge events. Click a bar to select. Left scale is badge count; right scale is fraction of network events. |
| `3` | Activity timeline | Selected employee's badge and network events by January day and time. Shaded bands show the common after-hours filter. |
| `4` | Geography | Original Flovania map with counts of accounts by city. |

`N` / `P` (or arrow keys) cycle through ranked people in view 1 and ranked employees in the other views. `E` exports CSVs and the findings report. `S` saves the current view as PNG. `Q` / Escape quits. Window resizing scales the plotting geometry; a 1440 × 920 window is recommended for the text.

The graph uses incoming and outgoing person-to-person relationships from `ID1 -> ID2`. Ego mode includes links between visible neighbors and shows arrowheads; the full overview omits arrowheads for readability. Overview positions are schematic and are not geographic coordinates. No force layout or implied organizational hierarchy is imposed.

Examples:

```sh
./build/vast-explorer --person 5 --employee 12
./build/vast-explorer --start-hour 8 --end-hour 19
./build/vast-explorer --screenshot --output reports
./build-core/vast-analyze --start-hour 8 --end-hour 19 --output reports-8-to-19
```

`--screenshot` renders all four views and exits; it still requires VTK and a working graphics/display session. GUI startup does not automatically export CSVs; press `E` or run `vast-analyze`.

## Reports and findings

The normal-hours window defaults to **07:00 inclusive through 20:00 exclusive**, consistently applied to both badge and network logs. Hours are configurable within one day. They are an analytical convention: the supplied introduction permits 24x7 employee access.

`vast-analyze` or `E` produces:

- `findings.md`: counts, employee comparison, methods, and interpretation limits.
- `employee_summary.csv`: per-employee badge counts, classified entries, network counts/fractions, and request/response bytes.
- `after_hours_badges.csv`: individual timestamps and access types for review.
- `after_hours_destinations.csv`: employee/destination/port counts and bytes, plus how many distinct source IPs contact each destination across all hours.
- `social_degrees.csv`: all people, names, cities, and directed degrees, ranked by incoming degree.
- `city_population.csv`: accounts per city in this sample, not census population.

For the supplied data and default hours:

- 6,000 people and 29,876 unique person-to-person links; 12 location/other links excluded.
- 60 employees, 10,162 badge events, and 115,414 network requests; all source IPs match employees.
- Account 5 (`@jantke`, Kannvic) has the highest in-degree, 445.
- Employee 12 has the most after-hours classified-room entries, 11.

These are reproducible observations, not proof of espionage. Degree alone cannot establish a leader or handler role. The datasets contain no identity mapping between social accounts and employees, so equal numeric IDs are **not joined**. The badge log has no building-exit events, so this project does not infer that an employee was absent while their assigned machine was active. Network bytes and shared hosts do not independently establish exfiltration or intent. Video and the detailed mini-challenge question pages are absent from this folder; the report covers the supplied evidence only.

## Implementation

- `src/Data.h`, `src/Data.cpp`: strict delimited-file loading, joins, graph degrees, aggregation, command-line options, and reports.
- `src/Explorer.cpp`: four VTK renderer panels, colored polydata geometry, node/bar picking, keyboard callbacks, JPEG map, and PNG capture.
- `src/analyze.cpp`: command-line reporting entry point.
- `tests/test_data.cpp`: parsing, timestamp validation, hour boundaries, dataset regression, graph-degree conservation, join conservation, configurable hours, and report generation.

The loaders handle quoted CSV fields, CRLF, the schema rows in M2, trailing empty TSV columns, and the headerless Flitter-name table (including its first account). Invalid records report their filename and line number. Unknown source IPs remain in network data with employee `-1` and are counted explicitly; unknown badge IDs are rejected. Timestamps are intentionally restricted to January 2008, the supplied challenge period.

## Verification status

Both executables were compiled with AppleClang 17 and Homebrew VTK 9.7.1. The CTest suite passed against the supplied datasets. Reports were generated locally and independently checked against the raw data. The GUI help command also runs successfully. A four-view screenshot smoke test was attempted, but this execution session could not create a macOS OpenGL context and the rendering process crashed. **Visual rendering and mouse/keyboard interaction remain unverified.** Run `./build/vast-explorer --data .` in a normal Mac Terminal session to open the application, or use `--screenshot` to check rendering of all four views.
