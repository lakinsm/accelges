DESCRIPTION = "Openmoko Gestures"
SECTION = "openmoko/utilities"

PN = "gestures"
PV = "0.1.0+svnr${SRCREV}"
PR = "r2"

SRC_URI = "svn://svn.projects.openmoko.org/svnroot;module=gestures;proto=https"

S = "${WORKDIR}/gestures"

inherit autotools

do_install_append() {
	install -d ${D}${sysconfdir}/gestures/
	install -c -D -m 644 ${S}/config/* ${D}${sysconfdir}/gestures/
}

