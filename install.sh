#!/usr/bin/env bash
# NOTE: When making changes, make sure to run the script through ShellCheck!
# @see https://github.com/koalaman/shellcheck

# Update bash options.
# @see https://www.gnu.org/software/bash/manual/html_node/Modifying-Shell-Behavior.html
# @see https://sipb.mit.edu/doc/safe-shell/
# Exit script on error.
set -o errexit
# Error on unset (non-special) parameter expansion.
set -o nounset
# Disable pathname expansion.
set -o noglob

readonly EXIT_CODE_GENERIC_ERROR=1

function build_plugin() {
    echo -e "\n\nBUILDING..."
    if [[ -z "${SOURCE_DIR:-}" || ! -d "${SOURCE_DIR}" ]]; then
        echo "No such directory: ${SOURCE_DIR}"
        exit $EXIT_CODE_GENERIC_ERROR
    fi
    BUILD_DIR="${SOURCE_DIR}/build"
    mkdir -p "${BUILD_DIR}"
    cd "${BUILD_DIR}"
    local USER_QT_PLUGINS
    local CMAKE_ARGS=("-DCMAKE_BUILD_TYPE=Release")
    if [[ -z "${RUN_AS_ADMIN}" ]]; then
        USER_QT_PLUGINS="${HOME}/.local/lib/qt/plugins"
        mkdir -p "${USER_QT_PLUGINS}"
        CMAKE_ARGS+=("-DKDE_INSTALL_QTPLUGINDIR=${USER_QT_PLUGINS}")
        QT_PLUGIN_PATH="${QT_PLUGIN_PATH:-}"
        if [[ -z "${QT_PLUGIN_PATH}" || "${QT_PLUGIN_PATH}" != *"/.local/lib/qt/plugins/"* ]]; then
            # Add the installation path to the QT_PLUGIN_PATH
            echo "export QT_PLUGIN_PATH=${USER_QT_PLUGINS}/:${QT_PLUGIN_PATH}" >> "${HOME}/.bashrc"
            export QT_PLUGIN_PATH="${USER_QT_PLUGINS}/:${QT_PLUGIN_PATH}"
        fi
        local USER_SERVICES="${HOME}/.local/share/kservices5"
        mkdir -p "${USER_SERVICES}"
        CMAKE_ARGS+=("-DKDE_INSTALL_KSERVICES5DIR=${USER_SERVICES}")

    else
        USER_QT_PLUGINS=$(kf5-config --qt-plugins)
        CMAKE_ARGS+=("-DKDE_INSTALL_QTPLUGINDIR=${USER_QT_PLUGINS}")
    fi
    cmake "${CMAKE_ARGS[@]}" ..

    echo -e "\nBUILD FINISHED!"
}

function install_plugin() {
    echo -e "\n\nINSTALLING..."
    $RUN_AS_ADMIN make -j$(nproc) install

    # Restart Krunner.
    kquitapp5 krunner 2>/dev/null
    kstart5 --windowclass krunner krunner >/dev/null 2>&1 &

    echo -e "\nINSTALL FINISHED!"
}

function delete_temp_dir() {
    if [[ -n "${TEMP_DIR:-}" && -d "${TEMP_DIR}" ]]; then
        echo "Deleting temporary directory:"
        echo " - ${TEMP_DIR}"
        rm -rf "${TEMP_DIR}"
    fi
}

function determine_firefox_install_path() {
    FIREFOX_INSTALL_PATH="${FIREFOX_INSTALL_PATH:-}"
    if [[ -z "${FIREFOX_INSTALL_PATH}" ]]; then
        echo "Searching for firefox install path..."
        local FIREFOX_INSTALL_DIR_CANDIDATES="
            /usr/lib64/firefox
            /usr/lib64/firefox-esr
            /usr/lib/firefox
            /usr/lib/firefox-esr
        "
        for CANDIDATE in ${FIREFOX_INSTALL_DIR_CANDIDATES}; do
            if [[ -d "${CANDIDATE}" ]]; then
                FIREFOX_INSTALL_PATH="${CANDIDATE}"
                break;
            fi
        done
        if [[ -z "${FIREFOX_INSTALL_PATH}" ]]; then
            echo "Could not determine the firefox install directory. Please provide it using:"
            echo "    export FIREFOX_INSTALL_PATH=/path/to/firefox"
            exit $EXIT_CODE_GENERIC_ERROR
        fi
    fi
    if [[ ! -d "${FIREFOX_INSTALL_PATH}" ]]; then
        echo "No such directory: ${FIREFOX_INSTALL_PATH}"
        exit $EXIT_CODE_GENERIC_ERROR
    fi
    if [[ ! -e "${FIREFOX_INSTALL_PATH}/firefox" ]]; then
        echo "No firefox binary in: ${FIREFOX_INSTALL_PATH}"
        exit $EXIT_CODE_GENERIC_ERROR
    fi
    echo "Found firefox in: ${FIREFOX_INSTALL_PATH}"
}

function handle_exit_signal() {
    echo -e "\n\nCLEANUP..."
    delete_temp_dir
}

function install_private_browsing_icon() {
    echo "Installing private browsing icon:"
    # The "omni.ja" archive contains an archive of various runtime files, including icons.
    # @see https://developer.mozilla.org/en-US/docs/Mozilla/About_omni.ja_%28formerly_omni.jar%29
    local OMNI_ARCHIVE_RELATIVE_PATH="browser/omni.ja"
    local OMNI_ARCHIVE_FULL_PATH="${FIREFOX_INSTALL_PATH}/${OMNI_ARCHIVE_RELATIVE_PATH}"
    echo " - extracting icon from: ${OMNI_ARCHIVE_FULL_PATH}"
    if [[ ! -e "${OMNI_ARCHIVE_FULL_PATH}" ]]; then
        echo "No such file: ${OMNI_ARCHIVE_FULL_PATH}"
        exit $EXIT_CODE_GENERIC_ERROR
    fi
    local FAVICON_NAME="favicon.svg"
    local FAVICON_PATH="chrome/browser/skin/classic/browser/privatebrowsing/${FAVICON_NAME}"
    unzip -d "${TEMP_DIR}" -j -q "${OMNI_ARCHIVE_FULL_PATH}" "${FAVICON_PATH}"
    if [[ "${FIREFOX_INSTALL_PATH}" == *firefox-esr ]]; then
        # Change Icon name for ESR installations.
        sed -i "s/Icon=.*/Icon=firefox-esr/" src/plasma-runner-firefoxprofilerunner.desktop
    fi
    # The icon name for private browsing profiles.
    local NEW_ICON_NAME="private_browsing_firefox.svg"
    # The icon path for private browsing profiles.
    local NEW_ICON_DIR="/usr/share/pixmaps"
    if [[ -z "${RUN_AS_ADMIN}" ]]; then
        NEW_ICON_DIR="$HOME/.local/share/pixmaps"
    fi
    echo " - saving icon as: ${NEW_ICON_DIR}/${NEW_ICON_NAME}"
    $RUN_AS_ADMIN mkdir -p "${NEW_ICON_DIR}"
    $RUN_AS_ADMIN mv "${TEMP_DIR}/${FAVICON_NAME}" "${NEW_ICON_DIR}/${NEW_ICON_NAME}"
}

function prepare_temp_dir() {
    local VENDOR_NAME="FirefoxProfileRunner"
    TEMP_DIR="$(mktemp --directory --suffix="-${VENDOR_NAME}" --tmpdir=/tmp)"
    echo "Using temporary directory:"
    echo " - ${TEMP_DIR}"
}

function prepare_project_repo_dir() {
    SOURCE_DIR="${PWD}"
    # Clone project if it is downloaded using curl
    if [[ ! -e "${SOURCE_DIR}/FirefoxProfileRunner.kdev4" ]]; then
        echo "Cloning project repo..."
        CMD_PATH="$(command -v git)"
        if [[ -z "${CMD_PATH}" ]]; then
            echo "Could not find git."
            exit $EXIT_CODE_GENERIC_ERROR
        fi
        SOURCE_DIR="${TEMP_DIR}/repo"
        git clone https://github.com/alex1701c/krunner-firefox "${SOURCE_DIR}"
        cd "${SOURCE_DIR}"
    fi
}

function verify_requirements() {
    local CMD_PATH=""
    set +o errexit
    CMD_PATH="$(command -v cmake)"
    if [[ -z "${CMD_PATH}" ]]; then
        echo "Could not find cmake."
        exit $EXIT_CODE_GENERIC_ERROR
    fi
    CMD_PATH="$(command -v kf5-config)"
    if [[ -z "${CMD_PATH}" ]]; then
        echo "Could not find kf5-config."
        exit $EXIT_CODE_GENERIC_ERROR
    fi
    CMD_PATH="$(command -v make)"
    if [[ -z "${CMD_PATH}" ]]; then
        echo "Could not find make."
        exit $EXIT_CODE_GENERIC_ERROR
    fi
    CMD_PATH="$(command -v sed)"
    if [[ -z "${CMD_PATH}" ]]; then
        echo "Could not find sed."
        exit $EXIT_CODE_GENERIC_ERROR
    fi
    CMD_PATH="$(command -v unzip)"
    if [[ -z "${CMD_PATH}" ]]; then
        echo "Could not find unzip."
        exit $EXIT_CODE_GENERIC_ERROR
    fi
    set -o errexit
}

function prepare_build_and_install() {
    echo "Preparing to build and install the plugin..."
    if [[ "$(whoami)" == "root" ]]; then
        echo "The project should not be built using root, installation will use sudo."
        echo "Set 'GLOBAL_PLUGIN=false' for a user installation."
        echo ""
        echo "Installation canceled!"
        exit 1
    fi

    # By default, the install script is executed with admin privileges.
    RUN_AS_ADMIN="sudo"
    if [[ "_${GLOBAL_PLUGIN:-}_" == "_false_" || "_${GLOBAL_PLUGIN:-}_" == "_FALSE_" ]]; then
        RUN_AS_ADMIN=""
    fi

    # Verify script requirements.
    verify_requirements

    # Register a cleanup function to be called on the EXIT signal. Regardless of the exit code.
    trap handle_exit_signal EXIT

    # Clone project repository if the script has been downloaded using curl/wget.
    prepare_temp_dir
    prepare_project_repo_dir

    # Extract and install private browsing icon.
    determine_firefox_install_path
    install_private_browsing_icon
}

# Only run the install function when the script is executed and not when it's sourced.
# @see https://www.gnu.org/software/bash/manual/html_node/Bash-Variables.html
if [[ ${FUNCNAME[0]:-} == "main" ]]; then
    prepare_build_and_install
    build_plugin
    install_plugin
fi
