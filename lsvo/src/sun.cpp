/* file from Radiance */
/*
 *           SOLAR CALCULATIONS
 *
 *               3/31/87
 *
 */

#include "CSkyV3.hxx"
#include  <math.h>

bool
cvthour(			/* convert hour string */
	const char  *hs, float &hour
)
{
  register const char  *cp = hs;
  
  while (isdigit(*cp)) cp++;
  if (*cp == ':'){
    hour = atof(hs) + atof(++cp)/60.0f;
  }
  else {
    fprintf(stderr, "WRONG time format, must be hour:min\n");
    return false;
  }

  return true;
}

int jdate(		/* Julian date (days into year) */
	int month,
	int day
)
{
	static short  mo_da[12] = {0,31,59,90,120,151,181,212,243,273,304,334};
	
	return(mo_da[month-1] + day);
}

namespace ArchLight
{

double CSky::stadj(		/* solar time adjustment from Julian date */
		   int  day, double meridian
 )
{
	return( 0.170 * sin( (4*M_PI/373) * (day - 80) ) -
		0.129 * sin( (2*M_PI/355) * (day - 8) ) +
		12 * (longitude - meridian) / M_PI );
}


double CSky::sdec(		/* solar declination angle from Julian date */
		  int  jd
)
{
	return( 0.4093 * sin( (2*M_PI/368) * (jd - 81) ) );
}


double CSky::salt(	/* solar altitude from solar declination and solar time */
	double sd,
	double st
)
{
	return( asin( sin(latitude) * sin(sd) -
			cos(latitude) * cos(sd) * cos(st*(M_PI/12)) ) );
}


double CSky::sazi(	/* solar azimuth from solar declination and solar time */
	double sd,
	double st
)
{
  double ret =	(-atan2( cos(sd)*sin(st*(M_PI/12)),
 			-cos(latitude)*sin(sd) -
 			sin(latitude)*cos(sd)*cos(st*(M_PI/12)) ) );

  /* change to opengl coordinate system */
  if(ret > 0)
    ret -= M_PI;
  else
    ret += M_PI;
  return ret;
}

}
