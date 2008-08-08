DESCRIPTION = "Openmoko Accelerometer-based Gestures"
SECTION = "openmoko/utilities"

PN = "accelges"
PV = "0.1.0+svnr${SRCREV}"
PR = "r7"
DEPENDS = "dbus dbus-glib libxrandr libnotify notification-daemon curl gtk+"
RDEPENDS = "dbus dbus-glib libxrandr libnotify notification-daemon libcurl gtk+"

SRC_URI = "svn://accelges.googlecode.com/svn;module=trunk;proto=https"

S = "${WORKDIR}/trunk"

inherit autotools update-rc.d

#PACKAGES += "${PN}-cmd"

#INITSCRIPT_PACKAGES = "${PN} ${PN}-cmd"
#INITSCRIPT_NAME_${PN} = "gesd-neo2"
#INITSCRIPT_PARAMS_${PN} = "defaults 70"
#INITSCRIPT_NAME_${PN}-cmd = "gesl" 
#INITSCRIPT_PARAMS_${PN}-cmd = "defaults 80"

INITSCRIPT_NAME = "gesd-neo2"
INITSCRIPT_PARAMS = "defaults 70"

do_install_append() {
	install -d ${D}${sysconfdir}/accelges/neo2
	install -c -D -m 644 ${S}/config/neo2/* ${D}${sysconfdir}/accelges/neo2
	install -d ${D}${sysconfdir}/accelges/neo3
	install -c -D -m 644 ${S}/config/neo3/* ${D}${sysconfdir}/accelges/neo3
	install -d ${D}${sysconfdir}/accelges/wii
	install -c -D -m 644 ${S}/config/wii/* ${D}${sysconfdir}/accelges/wii
	
	install -d ${D}${sysconfdir}/init.d
	install -c -D -m 755 ${S}/config/init.d/* ${D}${sysconfdir}/init.d/

	install -d ${D}${sysconfdir}/dbus-1/system.d
	install -c -D -m 644 ${S}/gesd/data/accelges.conf ${D}${sysconfdir}/dbus-1/system.d
}

FILES_${PN} += ${datadir}

