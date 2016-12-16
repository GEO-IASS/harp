/*
 * Copyright (C) 2015-2016 S[&]T, The Netherlands.
 *
 * This file is part of HARP.
 *
 * HARP is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * HARP is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HARP; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "harp-internal.h"
#include "harp-constants.h"
#include "harp-geometry.h"

/** Check whether two ranges overlap, and if this is the case an overlapping percentage is returned.
 * The ranges must consist of a positive minimum and maximum.
 *
 * The overlapping percentage is calculated with
 *
 *    delta_overlapping_range / min(delta_rangeA, delta_rangeB)
 *
 * where
 *
 *    delta_overlapping_range = max_range - min_range.
 *
 * and
 *
 *    delta_overlapping_range =
 *      max_overlapping_range - min_overlapping_range
 *
 * The following cases need to be considered:
 * --------------------------------------------
 *   1. No overlap,
 *      range A lies rightwards of range B
 *      'harp_overlapping_scenario_no_overlap_b_a'
 *
 *            A----A
 *       B--B
 *
 *       max range B < min range A
 *
 * --------------------------------------------
 *   2. No overlap,
 *      range A lies leftwards of range B
 *      'harp_overlapping_scenario_no_overlap_a_b'
 *
 *       A----A
 *               B--B
 *
 *       max range A < min range B
 *
 * --------------------------------------------
 *   3. Equal,
 *      range A is equal to range B
 *      'harp_overlapping_scenario_overlap_a_equals_b'
 *
 *       A----A
 *       B----B
 *
 *       min range A == min range B &&
 *       max range A == max range B
 *
 * --------------------------------------------
 *   4. Partial overlap,
 *      max range B lies outside range A
 *      'harp_overlapping_scenario_partial_overlap_a_b'
 *
 *       A----A     A----A
 *           B--B        B---B
 *
 *       min range A < min range B <= max range A &&
 *       min range B <= max range A < max range B
 *
 * --------------------------------------------
 *   5. Partial overlap:
 *      min range B lies outside range A
 *      'harp_overlapping_scenario_partial_overlap_b_a'
 *
 *          A----A       A----A
 *        B--B        B--B
 *
 *        min range B < min range A <= max range B &&
 *        min range A <= max range B < max range A
 *
 * --------------------------------------------
 *   6. Contain:
 *      range A contains range B
 *      'harp_overlapping_scenario_overlap_a_contains_b'
 *
 *         A----A   A----A  A----A
 *           B-B    B-B        B-B
 *
 *         min range B >= min range A &&
 *          max range B <= max range A
 *
 * --------------------------------------------
 *   7. Contain:
 *      range B contains range A
 *      'harp_overlapping_scenario_overlap_b_contains_a'
 *
 *          A--A     A---A       A---A
 *         B-----B   B------B  B-----B
 *
 *         min range A >= min range B &&
 *         max range A <= max range B
 */
int harp_determine_overlapping_scenario(double xmin_a, double xmax_a, double xmin_b, double xmax_b,
                                        harp_overlapping_scenario *new_overlapping_scenario)
{
    harp_overlapping_scenario overlapping_scenario = harp_overlapping_scenario_no_overlap_b_a;

    /* Make sure that elements in range A and B are in ascending order */
    if (xmax_a < xmin_a)
    {
        harp_set_error(HARP_ERROR_INVALID_ARGUMENT,
                       "arguments 'xmin_a' (%g) and 'xmax_a' (%g) for overlapping scenario must be in ascending order",
                       xmin_a, xmax_a);
        return -1;
    }
    if (xmax_b < xmin_b)
    {
        harp_set_error(HARP_ERROR_INVALID_ARGUMENT,
                       "arguments 'xmin_b' (%g) and 'xmax_b' (%g) for overlapping scenario must be in ascending order",
                       xmin_b, xmax_b);
        return -1;
    }

    /* Consider all overlapping cases ... */
    if (xmax_b < xmin_a)
    {
        /* There is no overlap, because range A lies rightwards of range B */
        overlapping_scenario = harp_overlapping_scenario_no_overlap_b_a;
        /* overlapping_percentage = 0.0; */
    }
    else if (xmax_a < xmin_b)
    {
        /* There is no overlap, because range A lies leftwards of range B */
        overlapping_scenario = harp_overlapping_scenario_no_overlap_a_b;
        /* overlapping_percentage = 0.0; */
    }
    else if (xmin_a == xmin_b && xmax_a == xmax_b)
    {
        /* The two ranges are exactly the same */
        overlapping_scenario = harp_overlapping_scenario_overlap_a_equals_b;
        /* overlapping_percentage = 100.0; */
    }
    else if (xmin_a < xmin_b && xmin_b <= xmax_a && xmin_b <= xmax_a && xmax_a < xmax_b)
    {
        /* There is a partial overlap between range A and B (min range B lies outside range A) */
        overlapping_scenario = harp_overlapping_scenario_partial_overlap_a_b;
        /* overlapping_percentage = 0.0; */
    }
    else if (xmin_b < xmin_a && xmin_a <= xmax_b && xmin_a <= xmax_b && xmax_b < xmax_a)
    {
        /* There is a partial overlap between range A and B (min range A lies outside range B) */
        overlapping_scenario = harp_overlapping_scenario_partial_overlap_b_a;
        /* overlapping_percentage = 0.0; */
    }
    else if (xmin_b >= xmin_a && xmax_b <= xmax_a)
    {
        /* Range A contains range B */
        overlapping_scenario = harp_overlapping_scenario_overlap_a_contains_b;
        /* overlapping_percentage = 100.0; */
    }
    else if (xmin_a >= xmin_b && xmax_a <= xmax_b)
    {
        /* Range B contains range A */
        overlapping_scenario = harp_overlapping_scenario_overlap_b_contains_a;
        /* overlapping_percentage = 100.0; */
    }
    else
    {
        harp_set_error(HARP_ERROR_INVALID_ARGUMENT,
                       "exception determining overlapping range: rangeA = [%11.3f, %11.3f]; rangeB = [%11.3f, %11.3f]",
                       xmin_a, xmax_a, xmin_b, xmax_b);
        return -1;
    }

    *new_overlapping_scenario = overlapping_scenario;
    return 0;
}

/** Calculate the fraction of the day
 * \param datetime   Datetime [s since 2000-01-01]
 * \return the fraction of the day [1]
 */
double harp_fraction_of_day_from_datetime(double datetime)
{
    double datetime_in_days = datetime / 86400.0;

    return datetime_in_days - floor(datetime_in_days);
}

/** Calculate the fraction of the year
 * \param datetime   Datetime [s since 2000-01-01]
 * \return the fraction of the year [1]
 */
double harp_fraction_of_year_from_datetime(double datetime)
{
    double datetime_in_years = datetime / (365.2422 * 86400.0);

    return datetime_in_years - floor(datetime_in_years);
}

/** Calculate the equation of time (EOT) angle
 * \param datetime   Datetime [s since 2000-01-01]
 * \return the equation of time angle [degree]
 */
static double get_equation_of_time_angle_from_datetime(double datetime)
{
    double eot_angle;
    double pi = (double)M_PI;
    double rad2deg = (double)CONST_RAD2DEG;
    double b0 = 0.0072;
    double b1 = -0.0528;
    double b2 = -0.0012;
    double b3 = -0.1229;
    double b4 = -0.1565;
    double b5 = -0.0041;
    double fraction_of_year;
    double eta;

    /* Calculate eta */
    fraction_of_year = harp_fraction_of_year_from_datetime(datetime);
    eta = 2.0 * pi * fraction_of_year;

    eot_angle =
        rad2deg * (b0 * cos(eta) + b1 * cos(2.0 * eta) + b2 * cos(3.0 * eta) + b3 * sin(eta) + b4 * sin(2.0 * eta) +
                   b5 * sin(3.0 * eta));
    return eot_angle;
}

/** Calculate the solar declination angle
 * \param datetime   Datetime [s since 2000-01-01]
 * \return the solar declination angle [degree]
 */
static double get_solar_declination_angle_from_datetime(double datetime)
{
    double solar_declination_angle;     /* Solar declination angle [degree] */
    double pi = (double)M_PI;
    double rad2deg = (double)CONST_RAD2DEG;
    double a0 = 0.006918;
    double a1 = -0.399912;
    double a2 = -0.006758;
    double a3 = -0.002697;
    double a4 = 0.070257;
    double a5 = 0.000907;
    double a6 = 0.001480;
    double fraction_of_year;
    double eta;

    /* Calculate eta */
    fraction_of_year = harp_fraction_of_year_from_datetime(datetime);
    eta = 2.0 * pi * fraction_of_year;

    solar_declination_angle =
        rad2deg * (a0 + a1 * cos(eta) + a2 * cos(2.0 * eta) + a3 * cos(3.0 * eta) + a4 * sin(eta) +
                   a5 * sin(2.0 * eta) + a6 * sin(3.0 * eta));
    return solar_declination_angle;
}

/** Convert (electromagnetic wave) wavelength to (electromagnetic wave) frequency
 * \param wavelength      Wavelength [nm]
 * \return the frequency [Hz]
 */
double harp_frequency_from_wavelength(double wavelength)
{
    /* Convert wavelength [nm] to [m] */
    /* frequency = c / wavelength */
    return 1.0e9 * (double)CONST_SPEED_OF_LIGHT / wavelength;
}

/** Convert (electromagnetic wave) wavenumber to (electromagnetic wave) frequency
 * \param wavenumber      Wavenumber [1/cm]
 * \return the frequency [Hz]
 */
double harp_frequency_from_wavenumber(double wavenumber)
{
    /* Convert wavenumber [1/cm] to [1/m] */
    /* frequency = c * wavenumber */
    return 1.0e2 * (double)CONST_SPEED_OF_LIGHT *wavenumber;
}

/* Calculate the gravitational acceleration gsurf at the Earth's surface for a given latitude
 * Using WGS84 Gravity formula
 * \param latitude  Latitude [degree_north]
 * \return the gravitational acceleration at the Earth's surface gsurf [m/s2] */
double harp_gravity_at_surface_from_latitude(double latitude)
{
    double g_e = 9.7803253359;
    double k = 0.00193185265241;
    double e2 = 0.00669437999013;
    double sinphi = sin(latitude * CONST_DEG2RAD);

    return g_e * (1 + k * sinphi * sinphi) / sqrt(1 - e2 * sinphi * sinphi);
}

/* Calculate the gravitational acceleration g for a given latitude and height.
 * Using WGS84 Gravity formula
 * \param latitude  Latitude [degree_north]
 * \param height    Height [m]
 * \return the gravitational acceleration at the Earth's surface gsurf [m/s2] */
double harp_gravity_from_latitude_and_height(double latitude, double height)
{
    double a = 6378137.0;
    double f = 1 / 298.257223563;
    double m = 0.00344978650684;
    double sinphi = sin(latitude * CONST_DEG2RAD);

    return harp_gravity_at_surface_from_latitude(latitude) *
        (1 - (2 * (1 + f + m - 2 * f * sinphi * sinphi) + 3 * height / a) * height / a);
}

/* Calculate the local curvature radius Rsurf at the Earth's surface for a given latitude
 * \param latitude  Latitude [degree_north]
 * \return the local curvature radius Rsurf [m] */
double harp_local_curvature_radius_at_surface_from_latitude(double latitude)
{
    double Rsurf;
    double deg2rad = (double)(CONST_DEG2RAD);
    double phi = latitude * deg2rad;
    double Rmin = 6356752.0;    /* [m] */
    double Rmax = 6378137.0;    /* [m] */

    Rsurf = 1.0 / sqrt(cos(phi) * cos(phi) / (Rmin * Rmin) + sin(phi) * sin(phi) / (Rmax * Rmax));
    return Rsurf;
}

/** Convert radiance to normalized radiance
 * \param radiance  Radiance [mW m-2 sr-1]
 * \param solar_irradiance  Solar irradiance [mW m-2]
 * \return the normalized radiance [1]
 */
double harp_normalized_radiance_from_radiance_and_solar_irradiance(double radiance, double solar_irradiance)
{
    return M_PI * radiance / solar_irradiance;
}

/** Convert reflectance to normalized radiance
 * \param reflectance  Reflectance [1]
 * \param solar_zenith_angle  Solar zenith angle [degree]
 * \return the normalized radiance [1]
 */
double harp_normalized_radiance_from_reflectance_and_solar_zenith_angle(double reflectance, double solar_zenith_angle)
{
    return cos(solar_zenith_angle * CONST_DEG2RAD) * reflectance;
}

/** Convert normalized radiance to radiance
 * \param normalized_radiance  Normalized radiance [1]
 * \param solar_irradiance  Solar irradiance [mW m-2]
 * \return the radiance [mW m-2 sr-1]
 */
double harp_radiance_from_normalized_radiance_and_solar_irradiance(double normalized_radiance, double solar_irradiance)
{
    double radiance;    /* Radiance [mW m-2 sr-1] */
    double pi = (double)M_PI;

    radiance = normalized_radiance * solar_irradiance / pi;
    return radiance;
}

/** Convert reflectance to radiance
 * \param reflectance  Reflectance [1]
 * \param solar_irradiance  Solar irradiance [mW m-2]
 * \param solar_zenith_angle  Solar zenith angle [degree]
 * \return the radiance [mW m-2 sr-1]
 */
double harp_radiance_from_reflectance_solar_irradiance_and_solar_zenith_angle(double reflectance,
                                                                              double solar_irradiance,
                                                                              double solar_zenith_angle)
{
    double radiance;    /* Radiance [mW m-2 sr-1] */
    double pi = (double)M_PI;
    double deg2rad = (double)CONST_DEG2RAD;
    double mu0 = cos(solar_zenith_angle * deg2rad);

    radiance = reflectance * mu0 * solar_irradiance / pi;
    return radiance;
}

/** Convert radiance to reflectance
 * \param radiance  Radiance [mW m-2 sr-1]
 * \param solar_irradiance  Solar irradiance [mW m-2]
 * \param solar_zenith_angle  Solar zenith angle [degree]
 * \return the reflectance [1]
 */
double harp_reflectance_from_radiance_solar_irradiance_and_solar_zenith_angle(double radiance,
                                                                              double solar_irradiance,
                                                                              double solar_zenith_angle)
{
    double reflectance; /* Reflectance [1] */
    double pi = (double)M_PI;
    double deg2rad = (double)CONST_DEG2RAD;
    double mu0 = cos(solar_zenith_angle * deg2rad);

    reflectance = pi * radiance / (mu0 * solar_irradiance);
    return reflectance;
}

/** Convert normalized radiance to reflectance
 * \param normalized_radiance  Normalized radiance [mW m-2 sr-1]
 * \param solar_zenith_angle  Solar zenith angle [degree]
 * \return the reflectance [1]
 */
double harp_reflectance_from_normalized_radiance_and_solar_zenith_angle(double normalized_radiance,
                                                                        double solar_zenith_angle)
{
    double reflectance; /* Reflectance [1] */
    double deg2rad = (double)CONST_DEG2RAD;
    double mu0 = cos(solar_zenith_angle * deg2rad);

    reflectance = normalized_radiance / mu0;
    return reflectance;
}

/** Convert sensor and solar angles into scattering angle
 * \param sensor_zenith_angle Sensor Zenith Angle [degree]
 * \param solar_zenith_angle Solar Zenith Angle [degree]
 * \param relative_azimuth_angle Relative Azimuth Angle [degree]
 * \return the scattering angle [degree]
 */
double harp_scattering_angle_from_sensor_and_solar_angles(double sensor_zenith_angle, double solar_zenith_angle,
                                                          double relative_azimuth_angle)
{
    return CONST_RAD2DEG * acos(-cos(sensor_zenith_angle * CONST_DEG2RAD) * cos(solar_zenith_angle * CONST_DEG2RAD) -
                                sin(sensor_zenith_angle * CONST_DEG2RAD) * sin(solar_zenith_angle * CONST_DEG2RAD) *
                                cos(relative_azimuth_angle * CONST_DEG2RAD));
}

/** Calculate the solar azimuth angle for the given time and location
 * \param datetime Datetime [s since 2000-01-01]
 * \param latitude Latitude [degree_north]
 * \param longitude Longitude [degree_east]
 * \param solar_elevation_angle Pointer to the variable where the solar elevation angle [degree] will be stored
 * \param solar_azimuth_angle Pointer to the variable where the solar elevation angle [degree] will be stored
 */
void harp_solar_angles_from_datetime_latitude_and_longitude(double datetime, double latitude, double longitude,
                                                            double *solar_elevation_angle, double *solar_azimuth_angle)
{
    double pi = (double)M_PI;
    double rad2deg = (double)(CONST_RAD2DEG);
    double deg2rad = (double)(CONST_DEG2RAD);
    double phi = latitude * deg2rad;
    double solar_declination_angle;
    double eot_angle;
    double fraction_of_day;
    double hour_angle;
    double omega;
    double cos_psi;
    double sin_psi;

    /* Calculate the solar declination angle [rad] */
    solar_declination_angle = deg2rad * get_solar_declination_angle_from_datetime(datetime);

    /* Calculate the equation of time angle [rad] */
    eot_angle = deg2rad * get_equation_of_time_angle_from_datetime(datetime);

    /* Calculate the fraction of the day */
    fraction_of_day = harp_fraction_of_day_from_datetime(datetime);

    /* Calculate the hour angle [rad] */
    hour_angle = fraction_of_day + longitude / 360.0 + (eot_angle - 12.0) / 24.0;
    omega = 2.0 * pi * hour_angle;

    /* Calculate solar elevation angle [rad] */
    *solar_elevation_angle = asin(sin(solar_declination_angle) * sin(phi) +
                                  cos(solar_declination_angle) * cos(phi) * cos(omega));

    /* Calculate the solar azimuth angle [degree] */
    if (*solar_elevation_angle == 0.0)
    {
        *solar_azimuth_angle = 0.0;
    }
    else
    {
        cos_psi = (-sin(solar_declination_angle) * cos(phi) +
                   cos(solar_declination_angle) * sin(phi) * cos(omega)) / cos(*solar_elevation_angle);
        sin_psi = cos(solar_declination_angle) * sin(omega) / cos(*solar_elevation_angle);

        /* Use the two argument function atan2 to compute atan(y/x) given y and x, but with a range (-PI, PI] */
        *solar_azimuth_angle = rad2deg * atan2(sin_psi, cos_psi);
    }

    *solar_elevation_angle = rad2deg * (*solar_elevation_angle);
}

/** Convert sensor and solar azimuth angles to relative azimuth angle
 * \param sensor_azimuth_angle Sensor azimuth angle[degree]
 * \param solar_azimuth_angle Solar azimuth angle[degree]
 * \return the relative azimuth angle [degree]
 */
double harp_relative_azimuth_angle_from_sensor_and_solar_azimuth_angles(double sensor_azimuth_angle,
                                                                        double solar_azimuth_angle)
{
    double angle = sensor_azimuth_angle - solar_azimuth_angle;

    while (angle < 0)
    {
        angle += 360;
    }
    while (angle >= 360)
    {
        angle -= 360;
    }

    if (angle > 180)
    {
        return 360 - angle;
    }

    return angle;
}

/** Convert zenith angle to elevation angle
 * \param zenith_angle Zenith angle[degree]
 * \return the elevation angle [degree]
 */
double harp_elevation_angle_from_zenith_angle(double zenith_angle)
{
    return 90.0 - zenith_angle;
}

/** Convert zenith angle to elevation angle
 * \param elevation_angle  elevation angle [degree]
 * \return the zenith angle [degree]
 */
double harp_zenith_angle_from_elevation_angle(double elevation_angle)
{
    return 90.0 - elevation_angle;
}

/** Convert viewing angle (zenith, elevation, or azimuth) to sensor angle
 * \param viewing_angle Viewing angle[degree]
 * \return the sensor angle [degree]
 */
double harp_sensor_angle_from_viewing_angle(double viewing_angle)
{
    return 180.0 - viewing_angle;
}

/** Convert sensor angle (zenith, elevation, or azimuth) to viewing angle
 * \param sensor_angle  sensor angle [degree]
 * \return the viewing angle [degree]
 */
double harp_viewing_angle_from_sensor_angle(double sensor_angle)
{
    return 180.0 - sensor_angle;
}

/** Convert the solar zenith angle, the sensor zenith angle and relative azimuth angle at one height to another height
 * \param source_altitude  Source altitude [m]
 * \param source_solar_zenith_angle  Solar zenith angle at source altitude [degree]
 * \param source_sensor_zenith_angle  Sensor zenith angle at source altitude [degree]
 * \param source_relative_azimuth_angle  Relative azimuth angle at source altitude [degree]
 * \param target_altitude  Target altitude [m]
 * \param new_target_solar_zenith_angle  Solar zenith angle at target altitude [degree]
 * \param new_target_sensor_zenith_angle  Sensor zenith angle at target altitude [degree]
 * \param new_target_relative_azimuth_angle  Relative azimuth angle at target altitude [degree]
 */
int harp_sensor_geometry_angles_at_altitude_from_other_altitude(double source_altitude,
                                                                double source_solar_zenith_angle,
                                                                double source_sensor_zenith_angle,
                                                                double source_relative_azimuth_angle,
                                                                double target_altitude,
                                                                double *new_target_solar_zenith_angle,
                                                                double *new_target_sensor_zenith_angle,
                                                                double *new_target_relative_azimuth_angle)
{
    double target_solar_zenith_angle;
    double target_sensor_zenith_angle;
    double target_relative_azimuth_angle;
    double Earth_radius = (double)(CONST_EARTH_RADIUS_WGS84_SPHERE);
    double deg2rad = (double)(CONST_DEG2RAD);
    double rad2deg = (double)(CONST_RAD2DEG);
    double theta0 = source_solar_zenith_angle * deg2rad;        /* Solar zenith angle [rad] */
    double thetaV = source_sensor_zenith_angle * deg2rad;       /* Sensor zenith angle [rad] */
    double deltaphi = source_relative_azimuth_angle * deg2rad;  /* Relative azimuth angle [rad] */
    double sintheta0 = sin(theta0);
    double costheta0 = cos(theta0);
    double sinthetaV = sin(thetaV);
    double cosdeltaphi = cos(deltaphi);
    double sintheta0k;
    double costheta0k;
    double sinthetaVk;
    double theta0k;
    double thetaVk;
    double sinbeta;
    double cosbeta;
    double cosdeltaphik;
    double deltaphik;
    double fk;
    int nadir;

    nadir = (source_sensor_zenith_angle == 0.0);

    if (nadir || (target_altitude == source_altitude))
    {
        /*  The output angles are identical to the input angles */
        target_solar_zenith_angle = source_solar_zenith_angle;
        target_sensor_zenith_angle = source_sensor_zenith_angle;
        if (source_relative_azimuth_angle > 180.0)
        {
            target_relative_azimuth_angle = 360.0 - source_relative_azimuth_angle;
        }
        else
        {
            target_relative_azimuth_angle = source_relative_azimuth_angle;
        }
    }
    else
    {
        /* Calculate the sensor zenith angles */
        fk = (Earth_radius + source_altitude) / (Earth_radius + target_altitude);
        sinthetaVk = fk * sinthetaV;
        thetaVk = asin(sinthetaVk);

        /* Calculate the polar angle beta between the lines
         * (Earth centre -- target_altitude) and (Earth centre -- source_altitude) */
        sinbeta = thetaVk - thetaV;
        cosbeta = sqrt(1.0 - sinbeta * sinbeta);

        /* Calculate the solar zenith angles */
        costheta0k = costheta0 * cosbeta + sintheta0 * sinbeta * cosdeltaphi;
        theta0k = acos(costheta0k);
        sintheta0k = sqrt(1.0 - costheta0k * costheta0k);

        /* Calculate the sensor azimuth angles */
        if (sintheta0k == 0.0)
        {
            /* The sun is in zenith, so the azimuth angle is arbitrary. Set to zero. */
            deltaphik = 0.0;
        }
        else
        {
            cosdeltaphik = (costheta0 - costheta0k * cosbeta) / (sintheta0k * sinbeta);
            deltaphik = M_PI - acos(cosdeltaphik);
            if (deltaphik > M_PI)
            {
                deltaphik = 2.0 * M_PI - deltaphik;
            }
        }

        target_solar_zenith_angle = theta0k * rad2deg;
        target_sensor_zenith_angle = thetaVk * rad2deg;
        target_relative_azimuth_angle = deltaphik * rad2deg;
    }

    *new_target_solar_zenith_angle = target_solar_zenith_angle;
    *new_target_sensor_zenith_angle = target_sensor_zenith_angle;
    *new_target_relative_azimuth_angle = target_relative_azimuth_angle;
    return 0;
}

/** Calculate the solar zenith angle, the sensor zenith angle, and the relative azimuth angle for the requested altitudes
 * \param altitude  Height corresponding to the input angles (reference altitude) [m]
 * \param solar_zenith_angle  Solar zenith angle at reference altitude [degree]
 * \param sensor_zenith_angle  Sensor zenith angle at reference altitude [degree]
 * \param relative_azimuth_angle  Relative azimuth angle at reference altitude [degree]
 * \param num_levels Number of levels
 * \param altitude_profile  Altitude profile [m]
 * \param solar_zenith_angle_profile  Solar zenith angles at profile altitudes [degree]
 * \param sensor_zenith_angle_profile  Sensor zenith angles at profile altitudes [degree]
 * \param relative_azimuth_angle_profile  Relative azimuth angles at profile altitudes [degree]
 */
int harp_sensor_geometry_angle_profiles_from_sensor_geometry_angles(double altitude,
                                                                    double solar_zenith_angle,
                                                                    double sensor_zenith_angle,
                                                                    double relative_azimuth_angle,
                                                                    long num_levels,
                                                                    const double *altitude_profile,
                                                                    double *solar_zenith_angle_profile,
                                                                    double *sensor_zenith_angle_profile,
                                                                    double *relative_azimuth_angle_profile)
{
    long k;

    if (altitude_profile == NULL)
    {
        harp_set_error(HARP_ERROR_INVALID_ARGUMENT, "altitude profile is empty (%s:%u)", __FILE__, __LINE__);
        return -1;
    }
    if (solar_zenith_angle_profile == NULL)
    {
        harp_set_error(HARP_ERROR_INVALID_ARGUMENT, "solar zenith angle profile is empty (%s:%u)", __FILE__, __LINE__);
        return -1;
    }
    if (sensor_zenith_angle_profile == NULL)
    {
        harp_set_error(HARP_ERROR_INVALID_ARGUMENT, "sensor zenith angle profile is empty (%s:%u)", __FILE__, __LINE__);
        return -1;
    }
    if (relative_azimuth_angle_profile == NULL)
    {
        harp_set_error(HARP_ERROR_INVALID_ARGUMENT, "relative azimuth angle profile is empty (%s:%u)", __FILE__,
                       __LINE__);
        return -1;
    }

    for (k = 0; k < num_levels; k++)
    {
        if (harp_sensor_geometry_angles_at_altitude_from_other_altitude(altitude,
                                                                        solar_zenith_angle,
                                                                        sensor_zenith_angle,
                                                                        relative_azimuth_angle,
                                                                        altitude_profile[k],
                                                                        &(solar_zenith_angle_profile[k]),
                                                                        &(sensor_zenith_angle_profile[k]),
                                                                        &(relative_azimuth_angle_profile[k])) != 0)
        {
            return -1;
        }
    }

    return 0;
}

/** Convert (electromagnetic wave) frequency to (electromagnetic wave) wavelength
 * \param frequency Frequency [Hz]
 * \return Wavelength [nm]
 */
double harp_wavelength_from_frequency(double frequency)
{
    /* Convert [m] to [nm] */
    return 1.0e9 * (double)CONST_SPEED_OF_LIGHT / frequency;
}

/** Convert (electromagnetic wave) wavenumber to (electromagnetic wave) wavelength
 * \param wavenumber Wavenumber [1/cm]
 * \return Wavelength [nm]
 */
double harp_wavelength_from_wavenumber(double wavenumber)
{
    /* Convert wavenumber [1/cm] to [1/nm] */
    return 1.0e-7 / wavenumber;
}

/** Convert (electromagnetic wave) frequency to (electromagnetic wave) wavenumber
 * \param frequency Frequency [Hz]
 * \return Wavenumber [1/cm]
 */
double harp_wavenumber_from_frequency(double frequency)
{
    /* Convert wavenumber [m] to [cm] */
    return 1.0e-2 * frequency / (double)CONST_SPEED_OF_LIGHT;
}

/** Convert (electromagnetic wave) wavelength to (electromagnetic wave) wavenumber
 * \param wavelength Wavelength [nm]
 * \return Wavenumber [1/cm]
 */
double harp_wavenumber_from_wavelength(double wavelength)
{
    /* Convert wavelength [1/nm] to [1/cm] */
    return 1.0e7 / wavelength;
}
