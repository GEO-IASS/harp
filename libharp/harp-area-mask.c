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

#include "harp-area-mask.h"

#define AREA_MASK_BLOCK_SIZE 1024
#define AREA_MASK_MAX_LINE_SIZE 1024

int harp_area_mask_new(harp_area_mask **new_area_mask)
{
    harp_area_mask *area_mask;

    area_mask = (harp_area_mask *)malloc(sizeof(harp_area_mask));
    if (area_mask == NULL)
    {
        harp_set_error(HARP_ERROR_OUT_OF_MEMORY, "out of memory (could not allocate %lu bytes) (%s:%u)",
                       sizeof(harp_area_mask), __FILE__, __LINE__);
        return -1;
    }

    area_mask->num_polygons = 0;
    area_mask->polygon = NULL;

    *new_area_mask = area_mask;
    return 0;
}

void harp_area_mask_delete(harp_area_mask *area_mask)
{
    if (area_mask != NULL)
    {
        if (area_mask->polygon != NULL)
        {
            long i;

            for (i = 0; i < area_mask->num_polygons; i++)
            {
                harp_spherical_polygon_delete(area_mask->polygon[i]);
            }

            free(area_mask->polygon);
        }

        free(area_mask);
    }
}

int harp_area_mask_add_polygon(harp_area_mask *area_mask, harp_spherical_polygon *polygon)
{
    if (area_mask->num_polygons % AREA_MASK_BLOCK_SIZE == 0)
    {
        harp_spherical_polygon **new_polygon = NULL;

        new_polygon = realloc(area_mask->polygon, (area_mask->num_polygons + AREA_MASK_BLOCK_SIZE)
                              * sizeof(harp_spherical_polygon *));
        if (new_polygon == NULL)
        {
            harp_set_error(HARP_ERROR_OUT_OF_MEMORY, "out of memory (could not allocate %lu bytes) (%s:%u)",
                           (area_mask->num_polygons + AREA_MASK_BLOCK_SIZE) * sizeof(harp_spherical_polygon *),
                           __FILE__, __LINE__);
            return -1;
        }

        area_mask->polygon = new_polygon;
    }

    area_mask->polygon[area_mask->num_polygons] = polygon;
    area_mask->num_polygons++;
    return 0;
}

int harp_area_mask_covers_point(const harp_area_mask *area_mask, const harp_spherical_point *point)
{
    long i;

    for (i = 0; i < area_mask->num_polygons; i++)
    {
        if (harp_spherical_polygon_contains_point(area_mask->polygon[i], point))
        {
            return 1;
        }
    }

    return 0;
}

int harp_area_mask_covers_area(const harp_area_mask *area_mask, const harp_spherical_polygon *area)
{
    long i;

    for (i = 0; i < area_mask->num_polygons; i++)
    {
        if (harp_spherical_polygon_spherical_polygon_relationship(area_mask->polygon[i], area, 0)
            == HARP_GEOMETRY_POLY_CONT)
        {
            return 1;
        }
    }

    return 0;
}

int harp_area_mask_intersects_area(const harp_area_mask *area_mask, const harp_spherical_polygon *area,
                                   double min_percentage)
{
    long i;

    for (i = 0; i < area_mask->num_polygons; i++)
    {
        int dummy;
        double percentage;

        if (harp_spherical_polygon_overlapping_percentage(area_mask->polygon[i], area, &dummy, &percentage) != 0)
        {
            continue;
        }

        if (percentage > min_percentage)
        {
            return 1;
        }
    }

    return 0;
}

static int is_blank_line(const char *str)
{
    while (*str != '\0' && isspace(*str))
    {
        str++;
    }

    return (*str == '\0');
}

static int parse_polygon(const char *str, harp_spherical_polygon **new_polygon)
{
    harp_spherical_point_array *point_array;
    harp_spherical_polygon *polygon;
    const char *mark;
    int length;

    if (harp_spherical_point_array_new(&point_array) != 0)
    {
        return -1;
    }

    while (1)
    {
        harp_spherical_point point;

        while (*str != '\0' && isspace(*str))
        {
            str++;
        }

        if (*str == '\0')
        {
            break;
        }

        mark = str;
        while (*str != ',' && !isspace(*str) && *str != '\0')
        {
            str++;
        }

        length = str - mark;
        if (harp_parse_double(mark, length, &point.lat, 0) != length || !harp_isfinite(point.lat))
        {
            harp_set_error(HARP_ERROR_INVALID_FORMAT, "invalid latitude '%.*s' (%s:%u)", length, mark, __FILE__,
                           __LINE__);
            harp_spherical_point_array_delete(point_array);
            return -1;
        }

        while (*str != '\0' && isspace(*str))
        {
            str++;
        }

        if (*str != ',')
        {
            harp_spherical_point_array_delete(point_array);
            return -1;
        }

        /* Skip the comma. */
        str++;

        while (*str != '\0' && isspace(*str))
        {
            str++;
        }

        mark = str;
        while (*str != ',' && !isspace(*str) && *str != '\0')
        {
            str++;
        }

        length = str - mark;
        if (harp_parse_double(mark, length, &point.lon, 0) != length || !harp_isfinite(point.lon))
        {
            harp_set_error(HARP_ERROR_INVALID_FORMAT, "invalid longitude '%.*s' (%s:%u)", length, mark, __FILE__,
                           __LINE__);
            harp_spherical_point_array_delete(point_array);
            return -1;
        }

        /* Skip the comma, if there is one. */
        if (*str == ',')
        {
            str++;
        }

        harp_spherical_point_rad_from_deg(&point);
        harp_spherical_point_check(&point);

        if (harp_spherical_point_array_add_point(point_array, &point) != 0)
        {
            harp_spherical_point_array_delete(point_array);
            return -1;
        }
    }

    /* Discard the last point of the polygon if it is equal to the first. */
    if (point_array->numberofpoints > 1)
    {
        if (harp_spherical_point_equal(&point_array->point[0], &point_array->point[point_array->numberofpoints - 1]))
        {
            harp_spherical_point_array_remove_point_at_index(point_array, point_array->numberofpoints - 1);
        }
    }

    if (harp_spherical_polygon_from_point_array(point_array, &polygon) != 0)
    {
        harp_spherical_point_array_delete(point_array);
        return -1;
    }

    /* Once the spherical polygon has been succesfully created, the point array can be deleted. */
    harp_spherical_point_array_delete(point_array);

    /* Check the polygon */
    if (!harp_spherical_polygon_check(polygon))
    {
        harp_set_error(HARP_ERROR_INVALID_FORMAT, "invalid polygon");
        harp_spherical_polygon_delete(polygon);
        return -1;
    }

    *new_polygon = polygon;
    return 0;
}

static int read_area_mask(FILE *stream, harp_area_mask **new_area_mask)
{
    harp_area_mask *area_mask;
    char line[AREA_MASK_MAX_LINE_SIZE];
    int read_header;
    long i;

    if (harp_area_mask_new(&area_mask) != 0)
    {
        return -1;
    }

    i = 1;
    read_header = 0;
    while (fgets(line, AREA_MASK_MAX_LINE_SIZE, stream) != NULL)
    {
        harp_spherical_polygon *polygon;

        /* Skip blank lines. */
        if (is_blank_line(line))
        {
            i++;
            continue;
        }

        /* Skip header. */
        if (!read_header)
        {
            read_header = 1;
            i++;
            continue;
        }

        if (parse_polygon(line, &polygon) != 0)
        {
            harp_add_error_message(" (line %lu)", i);
            harp_area_mask_delete(area_mask);
            return -1;
        }

        if (harp_area_mask_add_polygon(area_mask, polygon) != 0)
        {
            harp_spherical_polygon_delete(polygon);
            harp_area_mask_delete(area_mask);
            return -1;
        }

        i++;
    }

    if (ferror(stream) || !feof(stream))
    {
        harp_set_error(HARP_ERROR_FILE_READ, "read error");
        harp_area_mask_delete(area_mask);
        return -1;
    }

    *new_area_mask = area_mask;
    return 0;
}

int harp_area_mask_read(const char *filename, harp_area_mask **new_area_mask)
{
    FILE *stream;
    harp_area_mask *area_mask;

    if (filename == NULL)
    {
        harp_set_error(HARP_ERROR_INVALID_ARGUMENT, "filename is NULL");
        return -1;
    }

    stream = fopen(filename, "r");
    if (stream == NULL)
    {
        harp_set_error(HARP_ERROR_FILE_OPEN, "cannot open area mask file '%s'", filename);
        return -1;
    }

    if (read_area_mask(stream, &area_mask) != 0)
    {
        harp_add_error_message(" (while reading area mask file '%s')", filename);
        fclose(stream);
        return -1;
    }

    if (fclose(stream) != 0)
    {
        harp_set_error(HARP_ERROR_FILE_CLOSE, "cannot close area mask file '%s'", filename);
        harp_area_mask_delete(area_mask);
        return -1;
    }

    *new_area_mask = area_mask;
    return 0;
}
