set -e

function get_version() {
	cd /source
	scripts/version.sh
}

function run_build_command() {
	OS=$(echo $OS_PLATFORM |  cut -d '-' -f 1)

	# we sort longer strings in front, to make more specific items
	# first.

	egrep -e "^${OS}([^-]|$)" -e "^${OS_PLATFORM}" /source/dbld/build.manifest | sort -r | head -1 | while read os env cmdline; do
	        env=$(echo ${env} | tr ',' ' ')
		echo "Running build as: " env $env "$@" $cmdline
	        env $env "$@" $cmdline
	done
}

VERSION=$(get_version)
