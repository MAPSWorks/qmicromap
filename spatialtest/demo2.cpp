/*

demo2:

version 2.2, 2008 September 13

Author: Sandro Furieri a-furieri@lqt.it

------------------------------------------------------------------------------

this is a sample C source showing how to manipulate GEOMETRY
1. creating geometries
2. exploring geometries
3. enquiring their basic properties

------------------------------------------------------------------------------

This software is provided 'as-is', without any express or implied
warranty.  In no event will the author be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
these headers are required in order to support
SQLite/SpatialDB
*/

#include <geos_c.h>

#include <spatialite/sqlite3.h>
#include <spatialite/gaiageo.h>
#include <spatialite.h>

static const  char *
geom_type (int type)
{
/* utility funtion returning corresponding GeometryType as a string */
    static const char *name = "EMPTY / NULL GEOMETRY";
    if (type == GAIA_POINT)
	name = "POINT";
    if (type == GAIA_LINESTRING)
	name = "LINESTRING";
    if (type == GAIA_POLYGON)
	name = "POLYGON";
    if (type == GAIA_MULTIPOINT)
	name = "MULTIPOINT";
    if (type == GAIA_MULTILINESTRING)
	name = "MULTILINESTRING";
    if (type == GAIA_MULTIPOLYGON)
	name = "MULTIPOLYGON";
    if (type == GAIA_GEOMETRYCOLLECTION)
	name = "GEOMETRYCOLLECTION";
    return name;
}

static void out_wkt(gaiaGeomCollPtr ptr) {
    gaiaOutBuffer out_buf;
    gaiaOutBufferInitialize (&out_buf);
    gaiaOutWkt (&out_buf, ptr);
    if (out_buf.BufferSize)
      {
	  printf ("\n%s\n", out_buf.Buffer);
      }
    gaiaOutBufferReset (&out_buf);
}

static void
geometry_printout (gaiaGeomCollPtr geom)
{
/* utility function printing a generic Geometry object */
    gaiaPointPtr pt;
    gaiaLinestringPtr ln;
    gaiaPolygonPtr pg;
    gaiaRingPtr rng;
    int n_pts = 0;
    int n_lns = 0;
    int n_pgs = 0;
    int cnt;
    int iv;
    int ir;
    double x;
    double y;


/* we'll now count how many POINTs are there */
    pt = geom->FirstPoint;
    while (pt)
      {
	  n_pts++;
	  pt = pt->Next;
      }
/* we'll now count how many LINESTRINGs are there */
    ln = geom->FirstLinestring;
    while (ln)
      {
	  n_lns++;
	  ln = ln->Next;
      }
/* we'll now count how many POLYGONs are there */
    pg = geom->FirstPolygon;
    while (pg)
      {
	  n_pgs++;
	  pg = pg->Next;
      }



    if (n_pts)
      {
/* printing POINTs coords */
	  cnt = 0;
	  pt = geom->FirstPoint;
	  while (pt)
	    {
/* we'll now scan the linked list of POINTs */
		printf ("\t\t\tPOINT %d/%d x=%1.4lf y=%1.4lf\n",
			cnt, n_pts, pt->X, pt->Y);
		cnt++;
		pt = pt->Next;
	    }
      }


    if (n_lns)
      {
/* printing LINESTRINGs coords */
	  cnt = 0;
	  ln = geom->FirstLinestring;
	  while (ln)
	    {
/* we'll now scan the linked list of LINESTRINGs */
		printf ("\t\t\tLINESTRING %d/%d has %d vertices\n",
			cnt, n_lns, ln->Points);
		for (iv = 0; iv < ln->Points; iv++)
		  {
/* we'll now retrieve coords for each vertex */
		      gaiaGetPoint (ln->Coords, iv, &x, &y);
		      printf ("\t\t\t\tvertex %d/%d x=%1.4lf y=%1.4lf\n",
			      iv, ln->Points, x, y);
		  }
		cnt++;
		ln = ln->Next;
	    }
      }


    if (n_pgs)
      {
/* printing POLYGONs coords */
	  cnt = 0;
	  pg = geom->FirstPolygon;
	  while (pg)
	    {
/* we'll now scan the linked list of POLYGONs */
		printf ("\t\t\tPOLYGON %d/%d has %d hole%c\n",
			cnt, n_pgs, pg->NumInteriors,
			(pg->NumInteriors == 1) ? ' ' : 's');

/*
now we'll print out the Exterior ring
[surely a POLYGON has an Exterior ring
*/
		rng = pg->Exterior;
		printf ("\t\t\t\tExteriorRing has %d vertices\n", rng->Points);
		for (iv = 0; iv < rng->Points; iv++)
		  {
/* we'll now retrieve coords for each vertex */
		      gaiaGetPoint (rng->Coords, iv, &x, &y);
		      printf ("\t\t\t\t\tvertex %d/%d x=%1.4lf y=%1.4lf\n",
			      iv, rng->Points, x, y);
		  }

		for (ir = 0; ir < pg->NumInteriors; ir++)
		  {
/*
a POLYGON can contain any arbitrary number of Interior rings
[this including ZERO]
*/
		      rng = pg->Interiors + ir;
		      printf ("\t\t\t\tInteriorRing %d/%d has %d vertices\n",
			      ir, pg->NumInteriors, rng->Points);
		      for (iv = 0; iv < rng->Points; iv++)
			{
/* we'll now retrieve coords for each vertex */
			    gaiaGetPoint (rng->Coords, iv, &x, &y);
			    printf
				("\t\t\t\t\tvertex %d/%d x=%1.4lf y=%1.4lf\n",
				 iv, rng->Points, x, y);
			}
		  }

		cnt++;
		pg = pg->Next;
	    }
      }
}

int
main (int argc, char *argv[])
{
    gaiaGeomCollPtr   geo_pt = NULL;
    gaiaGeomCollPtr   geo_ln = NULL;
    gaiaGeomCollPtr   geo_pg = NULL;
    gaiaGeomCollPtr   geo_mpt = NULL;
    gaiaGeomCollPtr   geo_mln = NULL;
    gaiaGeomCollPtr   geo_mpg = NULL;
    gaiaGeomCollPtr   geo_coll = NULL;
    gaiaLinestringPtr line;
    gaiaPolygonPtr    polyg;
    gaiaRingPtr       ring;

/*
this demo does not require any DB connection to be established
*/

/*
Step #1
creating and checking a POINT Geometry
*/

/* we'll allocate a Geometry object */
    geo_pt = gaiaAllocGeomColl ();
/* then we insert a POINT, directly passing its coords */
    gaiaAddPointToGeomColl (geo_pt, 1.5, 2.75);
/* now we'll print the main attributes for this geometry */
    printf ("step#1: %s\t\tDimension=%d IsValid=%d\n",
	    geom_type (gaiaGeometryType (geo_pt)),
	    gaiaDimension (geo_pt), gaiaIsValid (geo_pt));
    geometry_printout (geo_pt);

/*
Step #2
creating and checking a LINESTRING Geometry
*/
    geo_ln = gaiaAllocGeomColl ();
/* then we insert a  LINESTRING, specifing how many vertices it contains */
    line = gaiaAddLinestringToGeomColl (geo_ln, 5);
/*
we've got a pointer referencing the linestring we've just inserted
now we'll set coords for each vertex
*/
    gaiaSetPoint (line->Coords, 0, 1.0, 1.0);
    gaiaSetPoint (line->Coords, 1, 2.0, 1.0);
    gaiaSetPoint (line->Coords, 2, 2.0, 2.0);
    gaiaSetPoint (line->Coords, 3, 100.0, 2.0);
    gaiaSetPoint (line->Coords, 4, 100.0, 100.0);

    printf ("\nstep#2: %s\tDimension=%d IsValid=%d\n",
	    geom_type (gaiaGeometryType (geo_ln)),
	    gaiaDimension (geo_ln), gaiaIsValid (geo_ln));
    geometry_printout (geo_ln);

/*
Step #3
creating and checking a POLYGON Geometry
*/
    geo_pg = gaiaAllocGeomColl ();
/*
then we insert a  POLYGON, specifing:
- how many vertices have to be allocated for the Exterior Ring
- how many Interior Rings it has
*/
    polyg = gaiaAddPolygonToGeomColl (geo_pg, 5, 2);
/*
we've got a pointer referencing the polygon we've just inserted
now we'll get a pointer referencing its Exterior ring
*/
    ring = polyg->Exterior;

/* now we'll set coord for each Exterior ring vertex */
    gaiaSetPoint (ring->Coords, 0, 0.0, 0.0);
    gaiaSetPoint (ring->Coords, 1, 50.0, 0.0);
    gaiaSetPoint (ring->Coords, 2, 50.0, 50.0);
    gaiaSetPoint (ring->Coords, 3, 0.0, 50.0);
/* please note: a Ring is a CLOSED figure, so last and first vertex has to be coincident */
    gaiaSetPoint (ring->Coords, 4, 0.0, 0.0);

/*
we'll now get a pointer referencing the FIRST interior ring,
specifing how vertices have to be allocated
*/
    ring = gaiaAddInteriorRing (polyg, 0, 5);
/* then setting coords for each Interior ring vertex */
    gaiaSetPoint (ring->Coords, 0, 40.0, 40.0);
    gaiaSetPoint (ring->Coords, 1, 41.0, 40.0);
    gaiaSetPoint (ring->Coords, 2, 41.0, 41.0);
    gaiaSetPoint (ring->Coords, 3, 40.0, 41.0);
    gaiaSetPoint (ring->Coords, 4, 40.0, 40.0);

/*
we'll now get a pointer referencing the SECOND interior ring,
specifing how vertices have to be allocated
*/
    ring = gaiaAddInteriorRing (polyg, 1, 5);
/* then setting coords for each Interior ring vertex */
    gaiaSetPoint (ring->Coords, 0, 30.0, 30.0);
    gaiaSetPoint (ring->Coords, 1, 31.0, 30.0);
    gaiaSetPoint (ring->Coords, 2, 31.0, 31.0);
    gaiaSetPoint (ring->Coords, 3, 30.0, 31.0);
    gaiaSetPoint (ring->Coords, 4, 30.0, 30.0);


    printf ("\nstep#3: %s\tDimension=%d IsValid=%d\n",
	    geom_type (gaiaGeometryType (geo_pg)),
	    gaiaDimension (geo_pg), gaiaIsValid (geo_pg));
    geometry_printout (geo_pg);

/*
Step #4
creating and checking a MULTIPOINT Geometry
*/
    geo_mpt = gaiaAllocGeomColl ();
/* then we'll insert some POINTs */
    gaiaAddPointToGeomColl (geo_mpt, 5.0, 5.0);
    gaiaAddPointToGeomColl (geo_mpt, 15.0, 5.0);
    gaiaAddPointToGeomColl (geo_mpt, 5.0, 15.0);
    gaiaAddPointToGeomColl (geo_mpt, 25.0, 5.0);
    gaiaAddPointToGeomColl (geo_mpt, 5.0, 25.0);
    printf ("\nstep#4: %s\tDimension=%d IsValid=%d\n",
	    geom_type (gaiaGeometryType (geo_mpt)),
	    gaiaDimension (geo_mpt), gaiaIsValid (geo_mpt));
    geometry_printout (geo_mpt);

/*
Step #5
creating and checking a MULTILINESTRING Geometry
*/
    geo_mln = gaiaAllocGeomColl ();

/* then we'll insert two LINESTRINGs */
    line = gaiaAddLinestringToGeomColl (geo_mln, 2);
    gaiaSetPoint (line->Coords, 0, 30.0, 10.0);
    gaiaSetPoint (line->Coords, 1, 10.0, 30.0);

    line = gaiaAddLinestringToGeomColl (geo_mln, 2);
    gaiaSetPoint (line->Coords, 0, 40.0, 50.0);
    gaiaSetPoint (line->Coords, 1, 50.0, 40.0);

    printf ("\nstep#5: %s\tDimension=%d IsValid=%d\n",
	    geom_type (gaiaGeometryType (geo_mln)),
	    gaiaDimension (geo_mln), gaiaIsValid (geo_mln));
    geometry_printout (geo_mln);

/*
Step #6
creating and checking a MULTIPOLYGON Geometry
*/
    geo_mpg = gaiaAllocGeomColl ();

/* then we'll insert two POLYGONs */
    polyg = gaiaAddPolygonToGeomColl (geo_mpg, 5, 0);
    ring = polyg->Exterior;
    gaiaSetPoint (ring->Coords, 0, 60.0, 60.0);
    gaiaSetPoint (ring->Coords, 1, 70.0, 60.0);
    gaiaSetPoint (ring->Coords, 2, 70.0, 70.0);
    gaiaSetPoint (ring->Coords, 3, 60.0, 70.0);
    gaiaSetPoint (ring->Coords, 4, 60.0, 60.0);

    polyg = gaiaAddPolygonToGeomColl (geo_mpg, 5, 0);
    ring = polyg->Exterior;
    gaiaSetPoint (ring->Coords, 0, 80.0, 80.0);
    gaiaSetPoint (ring->Coords, 1, 90.0, 80.0);
    gaiaSetPoint (ring->Coords, 2, 90.0, 90.0);
    gaiaSetPoint (ring->Coords, 3, 80.0, 90.0);
    gaiaSetPoint (ring->Coords, 4, 80.0, 80.0);

    printf ("\nstep#6: %s\tDimension=%d IsValid=%d\n",
	    geom_type (gaiaGeometryType (geo_mpg)),
	    gaiaDimension (geo_mpg), gaiaIsValid (geo_mpg));
    geometry_printout (geo_mpg);

/*
Step #7
creating and checking a GEOMETRYCOLLECTION Geometry
*/
    geo_coll = gaiaAllocGeomColl ();

/* then we'll insert two POINTs */
    gaiaAddPointToGeomColl (geo_coll, 100.0, 100.0);
    gaiaAddPointToGeomColl (geo_coll, 100.0, 0.0);

/* then we'll insert two LINESTRINGs */
    line = gaiaAddLinestringToGeomColl (geo_coll, 2);
    gaiaSetPoint (line->Coords, 0, 130.0, 110.0);
    gaiaSetPoint (line->Coords, 1, 110.0, 130.0);

    line = gaiaAddLinestringToGeomColl (geo_coll, 2);
    gaiaSetPoint (line->Coords, 0, 140.0, 150.0);
    gaiaSetPoint (line->Coords, 1, 150.0, 140.0);

/* then we'll insert two POLYGONs */
    polyg = gaiaAddPolygonToGeomColl (geo_coll, 5, 0);
    ring = polyg->Exterior;
    gaiaSetPoint (ring->Coords, 0, 160.0, 160.0);
    gaiaSetPoint (ring->Coords, 1, 170.0, 160.0);
    gaiaSetPoint (ring->Coords, 2, 170.0, 170.0);
    gaiaSetPoint (ring->Coords, 3, 160.0, 170.0);
    gaiaSetPoint (ring->Coords, 4, 160.0, 160.0);

    polyg = gaiaAddPolygonToGeomColl (geo_coll, 5, 0);
    ring = polyg->Exterior;
    gaiaSetPoint (ring->Coords, 0, 180.0, 180.0);
    gaiaSetPoint (ring->Coords, 1, 190.0, 180.0);
    gaiaSetPoint (ring->Coords, 2, 190.0, 190.0);
    gaiaSetPoint (ring->Coords, 3, 180.0, 190.0);
    gaiaSetPoint (ring->Coords, 4, 180.0, 180.0);

    printf ("\nstep#7: %s\tDimension=%d IsValid=%d\n",
	    geom_type (gaiaGeometryType (geo_coll)),
	    gaiaDimension (geo_coll), gaiaIsValid (geo_coll));
    geometry_printout (geo_coll);

/*
Step #8
printing each geometry as WKT
*/
   printf ("\nstep#8: checking WKT representations\n");

/* first we'll get the WKT correspondig to geometry */
    out_wkt(geo_pt);
    out_wkt(geo_ln);
    out_wkt(geo_pg);
    out_wkt(geo_mpt);
    out_wkt(geo_mln);
    out_wkt(geo_mpg);
    out_wkt(geo_coll);

/*
memory cleanup
we have to destroy each object using temporary storage before exit
*/
    if (geo_pt)
	gaiaFreeGeomColl (geo_pt);
    if (geo_ln)
	gaiaFreeGeomColl (geo_ln);
    if (geo_pg)
	gaiaFreeGeomColl (geo_pg);
    if (geo_mpt)
	gaiaFreeGeomColl (geo_mpt);
    if (geo_mln)
	gaiaFreeGeomColl (geo_mln);
    if (geo_mpg)
	gaiaFreeGeomColl (geo_mpg);
    if (geo_coll)
	gaiaFreeGeomColl (geo_coll);
    return 0;
}
