/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.

   Please see https://computerenhance.com for more information

   ======================================================================== */

/* ========================================================================
   LISTING 65
   ======================================================================== */
#include <math.h>

#include "haversine_types.h"

#include "replacement_math.cpp"

// NOTE(casey): EarthRadius is generally expected to be 6372.8
static f64 ReferenceHaversineR1(f64 X0, f64 Y0, f64 X1, f64 Y1, f64 EarthRadius)
{
    f64 lat1 = Y0;
    f64 lat2 = Y1;
    f64 lon1 = X0;
    f64 lon2 = X1;

    f64 dLat = RadiansFromDegrees(lat2 - lat1);
    f64 dLon = RadiansFromDegrees(lon2 - lon1);
    lat1 = RadiansFromDegrees(lat1);
    lat2 = RadiansFromDegrees(lat2);

    f64 a = Square(SinCE(dLat/2.0)) + CosCE(lat1)*CosCE(lat2)*Square(SinCE(dLon/2));
    f64 c = 2.0*ASinCE(SqrtCE(a));

    f64 Result = EarthRadius * c;

    return Result;
}

static inline f64 ReferenceHaversineR2(f64 X0, f64 Y0, f64 X1, f64 Y1, f64 EarthRadius)
{
    f64 dLat = RadiansFromDegrees(Y1 - Y0);
    f64 dLon = RadiansFromDegrees(X1 - X0);

    f64 a = Square(SinCE(dLat/2.0)) + CosCE(RadiansFromDegrees(Y0))*CosCE(RadiansFromDegrees(Y1))*Square(SinCE(dLon/2));
    return ASinCE(SqrtCE(a));
}
