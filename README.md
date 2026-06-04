# amnezia-client-tests

Out-of-tree test suite for [`coffeegrind123/amnezia-client`](https://github.com/coffeegrind123/amnezia-client).

This mirrors upstream's approach (amnezia-vpn PR #2550, "move tests to a
separate repo"): the client repo ships no tests, and this repo holds the
QtTest suite plus the CI that builds the client and runs it.

## Why a separate repo

Keeping tests here means the client tree stays byte-identical to upstream in
the test-affected files, so pulling `upstream/dev` never conflicts on a
`client/tests/` directory.

Tests reach the client's internals through the `protected:` getters on
`CoreController` (added upstream in #2550) by subclassing it — see
[`tests/testableCoreController.h`](tests/testableCoreController.h). No `friend`
declarations are needed in the client for these. The masterdnsvpn engine tests
are the one exception: they use `friend class ::TestMasterDnsVpnEngine`, which
is declared in the client's own `client/masterdnsvpn/{arq,resolverpool}.h`
(fork-only code) and works across repos because `friend` only names a global
class.

## Layout

```
tests/                         QtTest sources, CMakeLists.txt (built as a client subdir)
  testableCoreController.h     subclass exposing CoreController's protected getters
  testServerRepositoryHelpers.h
  test*.cpp                    one QTEST_MAIN per file
scripts/
  run-tests.sh                 clone client -> inject tests -> configure -> build -> ctest
  inject_tests.py             idempotently hooks add_subdirectory(tests) into the client
.github/workflows/tests.yml    Linux CI
```

The suite is built as a **subdirectory of the client** so it inherits the
parent build's `${SOURCES}`, `${HEADERS}`, `${LIBS}` and `${CLIENT_ROOT_DIR}`.
`scripts/inject_tests.py` inserts `add_subdirectory(tests)` *before* `main.cpp`
is appended to `${SOURCES}`, so `test_common` (compiled from `${SOURCES}`) does
not pull in the app's `main()` and clash with each test's `QTEST_MAIN`.

## Running locally

Requires Qt 6 (with `qtremoteobjects`, `qt5compat`, `qtshadertools`), CMake,
Ninja, a C++ compiler, Python 3, and conan 2.x.

```bash
# defaults: CLIENT_REF=dev, BUILD_TYPE=Debug, Ninja
QT_TOOLCHAIN=/path/to/Qt/6.8.x/gcc_64/lib/cmake/Qt6/qt.toolchain.cmake \
  bash scripts/run-tests.sh

# test a specific client commit/branch
CLIENT_REF=my-feature-branch bash scripts/run-tests.sh
```

Headless GUI/QML tests need `QT_QPA_PLATFORM=offscreen` (CI sets this).

## CI

`.github/workflows/tests.yml` runs on push / PR and via manual dispatch
(`workflow_dispatch`), where you can pass a `client_ref` to test the suite
against any client branch/tag/sha.

## Test inventory

| Executable | Source |
|---|---|
| `test_import_export` | testAdminSelfHostedExport.cpp |
| `test_multiple_imports` | testMultipleImports.cpp |
| `test_server_edit` | testServerEdit.cpp |
| `test_default_server_change` | testDefaultServerChange.cpp |
| `test_server_edge_cases` | testServerEdgeCases.cpp |
| `test_signal_order` | testSignalOrder.cpp |
| `test_servers_model_sync` | testServersModelSync.cpp |
| `test_complex_operations` | testComplexOperations.cpp |
| `test_settings_signals` | testSettingsSignals.cpp |
| `test_ui_servers_model_and_controller` | testUiServersModelAndController.cpp |
| `test_self_hosted_server_setup` | testSelfHostedServerSetup.cpp |
| `test_master_dns_vpn_config` | testMasterDnsVpnConfig.cpp |
| `test_master_dns_vpn_engine` | testMasterDnsVpnEngine.cpp |
