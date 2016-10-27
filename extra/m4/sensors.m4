dnl 08/23/2016

dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 2 of the License, or
dnl (at your option) any later version.

dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.

dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write to the Free Software
dnl Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
dnl MA 02110-1301, USA.


dnl TEST_SENSORS() function in configure.ac
dnl
dnl Substitute the linker flags -lsensors to the
dnl the variable 'SENSORS_LIBS' if the user enabled
dnl the --with-sensors switch
AC_DEFUN([TEST_SENSORS],[
  SENSORS_LIBS=""

  AC_ARG_WITH([sensors],
    AS_HELP_STRING([--with-sensors],
      [lm sensors linker flag to obtain voltage, mobo temp and fans]),
    [],
    [with_sensors=no]
  )

  ifdef([LINUKS],[
    AS_IF([test "x$with_sensors" = "xyes"], [
      AC_CHECK_HEADERS([sensors/sensors.h], [
        SENSORS_LIBS="-lsensors"
        ],[
          ERR_MUST_INSTALL([lm_sensors])
      ])

      m4_foreach([LiB], [
          sensors_init,
          sensors_get_value,
          sensors_get_label,
          sensors_get_detected_chips,
          sensors_get_features,
          sensors_get_all_subfeatures,
          sensors_cleanup
        ],[
          AC_CHECK_LIB(sensors,LiB,[],[
            MISSING_FUNC()
          ])
      ])
    ])
  ],[
  ])

  ifdef([FREEBZD],[
    AC_ARG_VAR(mobo_sensor, [mobo sensors module name to use in sysctl calls])
    if [[ ! -z "${mobo_sensor}" ]]
    then
      MOBO_MODL=\""${mobo_sensor}"\"
      AC_DEFINE_UNQUOTED([MOBO_MODL],[$MOBO_MODL],[mobo sensors module])
    fi

    AC_ARG_VAR(cpu_sensor, [cpu sensors module name to use in sysctl calls])
    if [[ ! -z "${cpu_sensor}" ]]
    then
      CPU_MODL=\""${cpu_sensor}"\"
      AC_DEFINE_UNQUOTED([CPU_MODL],[$CPU_MODL],[cpu sensors module])
    fi

    GOT_APM=0
    AC_ARG_WITH([apm],
      AS_HELP_STRING([--with-apm],
        [APM power and resource management for laptops]),
      [],
      [with_apm=no]
    )
    AS_IF([test "x$with_apm" = "xyes"], [
      AC_CHECK_HEADERS([machine/apm_bios.h], [
        GOT_APM=1
        ],[
          MISSING_HEADER()
      ])
    ])
    AC_DEFINE_UNQUOTED([GOT_APM],[$GOT_APM],[APM power and resource management for laptops])

  ],[
  ])

  AC_SUBST(SENSORS_LIBS)

  ifdef([LINUKS],[
    AS_IF([test "x$with_sensors" = "xyes"], [
      AC_LINK_IFELSE([
        AC_LANG_SOURCE([[
          #include <stdio.h>
          #include <string.h>
          #include <stdlib.h>
          #include <sensors/sensors.h>
          int main(void) {
            int nr = 0, nr2 = 0, nr3 = 0;
            const sensors_chip_name *chip;
            const sensors_feature *features;
            const sensors_subfeature *subfeatures;
            double value = 0.0;
            char *label = NULL;

            if (0 != (sensors_init(NULL))) {
              return 0;
            }

            while (NULL != (chip = sensors_get_detected_chips(NULL, &nr))) {
              nr2 = 0;
              while (NULL != (features = sensors_get_features(chip, &nr2))) {
                nr3 = 0;
                while (NULL != (subfeatures = sensors_get_all_subfeatures(chip, features, &nr3))) {
                  switch(subfeatures->type) {

                    case SENSORS_SUBFEATURE_IN_INPUT:
                    case SENSORS_SUBFEATURE_TEMP_INPUT:
                    case SENSORS_SUBFEATURE_FAN_INPUT:
                    {
                      if (0 != (sensors_get_value(chip, subfeatures->number, &value))) {
                        break;
                      }
                      if (NULL == (label = sensors_get_label(chip, features))) {
                        break;
                      }
                      printf("%f\n", (float)value);
                      if (NULL != label) {
                        free(label);
                      }
                    }
                      break;
                    default:
                      continue;
                  }
                }
              }
            }
            sensors_cleanup();

            return 0;
          }
        ]])
      ],[],[
          ERR([Either you miss lm_sensors or your lm_sensors API version is unsupported by pinky-bar.])
        ]
      )
    ])

  ],[
  ])


  ifdef([OPENBZD], [
    AC_COMPILE_IFELSE([
      AC_LANG_SOURCE([[
        #include <stdio.h>
        #include <stdlib.h>
        #include <string.h>
        #include <sys/sensors.h>
        int main(void) {
          struct sensordev dev;
          struct sensor sens;
          SENSOR_VOLTS_DC;
          SENSOR_TEMP;
          SENSOR_FANRPM;
          SENSOR_FINVALID;
          SENSOR_FUNKNOWN;
          return 0;
        }
      ]])
    ],[],[
      COMPILE_FAILED([sensors])
      ]
    )
  ],[
  ])

  AS_IF([test "x$with_sensors" = "xyes"], [
    ifdef([NO_PCI],[
      CHECK_CFLAGZ([-O0])
    ],[])
    m4_define([GOT_SENSR],[meltCPU])
  ])

])
