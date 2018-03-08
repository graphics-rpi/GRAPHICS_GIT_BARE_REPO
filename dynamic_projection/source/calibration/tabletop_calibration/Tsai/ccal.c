/*******************************************************************************\
*                                                                               *
* This program reads in a file containing coplanar calibration data             *
* and then uses the routines in cal_main.c to perform Tsai's                    *
* coplanar camera calibration.                                                  *
*                                                                               *
* If additional calibration data files are included on the command line         *
* they are tested using the model calibrated from the first set of              *
* calibration data.                                                             *
*                                                                               *
* At the end of the program the calibrated camera model is dumped in a format   *
* that can be loaded in by other programs.                                      *
*                                                                               *
* History                                                                       *
* -------                                                                       *
*                                                                               *
* 01-Apr-95  Reg Willson (rgwillson@mmm.com) at 3M St. Paul, MN                 *
*       Filename changes for DOS port.                                          *
*                                                                               *
* 30-Nov-93  Reg Willson (rgw@cs.cmu.edu) at Carnegie-Mellon University         *
*       Updated to use new calibration statistics routines.                     *
*                                                                               *
* 01-May-93  Reg Willson (rgw@cs.cmu.edu) at Carnegie-Mellon University         *
*       Modified to use utility routines.                                       *
*                                                                               *
* 07-Feb-93  Reg Willson (rgw@cs.cmu.edu) at Carnegie-Mellon University         *
*       Original implementation.                                                *
*                                                                               *
\*******************************************************************************/

#include <stdio.h>
#include <stdlib.h> /* tcy 8/22/08 */
#include "cal_main.h"

main (argc, argv)
    int       argc;
    char    **argv;
{
    FILE     *data_fd;

    int       i;

    if (argc != 8) {
	fprintf (stderr, "syntax: %s <input file> <width> <height> <output file> <xoff> <yoff> <zoff>\n", argv[0]);
	exit (-1);
    }

    /* initialize the camera parameters (cp) with the appropriate camera constants */
    initialize_projector_parms (); // tcy 9/2/08

    fprintf (stderr, "\nNonCoplanar calibration (Tz, f, kappa1 optimization) \n");

    fprintf (stderr, "\ncamera type: %s\n", camera_type);

    /* run through all of the files on the command line */
    //    for (i = 1; i < argc; i++) {
	if ((data_fd = fopen (argv[1], "r")) == NULL) {
	    fprintf (stderr, "%s: unable to open file \"%s\"\n", argv[0], argv[1]);
	    exit (-1);
	}
	/* load up the calibration data (cd) from the given data file */
	load_cd_data (data_fd, &cd);
	fclose (data_fd);

        fprintf (stderr, "\ndata file: %s  (%d points)\n\n", argv[1], cd.point_count);

        //	if (i == 1) {
	    /* determine the calibration constants from the 1st data file */
          //coplanar_calibration ();
                       coplanar_calibration_with_full_optimization ();
          //noncoplanar_calibration ();
          //noncoplanar_calibration_with_full_optimization ();

	    print_cp_cc_data (stderr, &cp, &cc);
            //	}

	print_error_stats (stderr);
        // }

    dump_cp_cc_data (stdout, &cp, &cc);

    int width = atoi(argv[2]);
    int height = atoi(argv[3]);
    // write the condolidated calibration file
    FILE *fp = fopen(argv[4], "wt");
    if (NULL == fp){
      fprintf(stderr, "unable to open %s\n", argv[4]);
      return -1;
    }

    fprintf(fp, "# width height\n");
    fprintf(fp, "%d %d\n", width, height);

    // intrinsics
    fprintf(fp, "# intrinsics matrix (K)\n");
    fprintf(fp,
            "%15.10lf %15.10lf %15.10lf\n"
            "%15.10lf %15.10lf %15.10lf\n"
            "%15.10lf %15.10lf %15.10lf\n",
            cc.f/cp.dx, 0., cp.Cx,
            0., cc.f/cp.dy, cp.Cy,
            0., 0., 1.);

    // extrinsics [R|t]
    fprintf(fp, "# extrinsics matrix ([R|t])\n");
    fprintf(fp,
            "%15.10lf %15.10lf %15.10lf %15.10lf\n"
            "%15.10lf %15.10lf %15.10lf %15.10lf\n"
            "%15.10lf %15.10lf %15.10lf %15.10lf\n",
            cc.r1, cc.r2, cc.r3, cc.Tx,
            cc.r4, cc.r5, cc.r6, cc.Ty,
            cc.r7, cc.r8, cc.r9, cc.Tz);

    fprintf(fp, "# sx\n");
    fprintf(fp, "%15.10lf\n", cp.sx);

    fprintf(fp, "# k1\n");
    fprintf(fp, "%15.10lf\n", cc.kappa1);

    double x_offset = atof(argv[5]);
    double y_offset = atof(argv[6]);
    double z_offset = atof(argv[7]);
    fprintf(fp, "# world coordinate frame offset\n");
    fprintf(fp, "%15.10lf %15.10lf %15.10lf\n", x_offset, y_offset, z_offset);
    fclose(fp);

    return 0;
}
