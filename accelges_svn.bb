DESCRIPTION = "Openmoko Accelerometer-based Gestures"
SECTION = "openmoko/utilities"

PN = "accelges"
PV = "0.1.0+svnr${SRCREV}"
PR = "r9"

SRC_URI = "svn://accelges.googlecode.com/svn;module=trunk;proto=https"

S = "${WORKDIR}/trunk"

inherit autotools

do_install_append() {
	install -d ${D}${sysconfdir}/accelges/neo2
	install -c -D -m 644 ${S}/config/neo2/* ${D}${sysconfdir}/accelges/neo2
	install -d ${D}${sysconfdir}/accelges/neo3
	install -c -D -m 644 ${S}/config/neo3/* ${D}${sysconfdir}/accelges/neo3
	install -d ${D}${sysconfdir}/accelges/wii
	install -c -D -m 644 ${S}/config/wii/* ${D}${sysconfdir}/accelges/wii
}

FILES_${PN} += ${datadir}

