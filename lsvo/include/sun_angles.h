#define  SUNEFFICACY		208.

double sdec(		/* solar declination angle from Julian date */
		  int  jd
)
{
	return( 0.4093 * sin( (2*M_PI/368) * (jd - 81) ) );
}

double stadj(		/* solar time adjustment from Julian date */
		   int  day, double meridian, double longitude
 )
{
	return( 0.170 * sin( (4*M_PI/373) * (day - 80) ) -
		0.129 * sin( (2*M_PI/355) * (day - 8) ) +
		12 * (longitude - meridian) / M_PI );
}

double salt(	/* solar altitude from solar declination and solar time */
	double sd,
	double st,
  double latitude
)
{
	return( asin( sin(latitude) * sin(sd) -
			cos(latitude) * cos(sd) * cos(st*(M_PI/12)) ) );
}


double sazi(	/* solar azimuth from solar declination and solar time */
	double sd,
	double st,
  double latitude
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

void polar2vec(float3 &v, float altitude, float azimuth)
{// radius=1
  v.y = sin(altitude);
  float r = cos(altitude);
  v.z = fabs(r*cos(azimuth));
  if(fabs(azimuth) < M_PI/2)
    v.z = -v.z;
  v.x = fabs(r*sin(azimuth));
  if(azimuth < 0)
    v.x = -v.x;
}

void calcSunPosition(float3& sundir, double lat, double lon, float dayOfYear, float timeOfDay, float& altitudeParam, double north)
{

  double latitude = lat;
  double longitude = lon;
  // int timeZone = -5;  //(eastern)
  // float meridian = timeZone * 15 * (M_PI/180.0); // just use this, not the same as Radiance
  
  float meridian =  15 * (M_PI/180.0); // just use this, not the same as Radiance

  double  sd, st;
  /* solar declination */
  sd = sdec(dayOfYear);

  st = timeOfDay + stadj(dayOfYear, meridian, longitude);
  //printf("time ofday %f \n", timeOfDay);
  //printf("st %lf sd %lf \n", st, sd);
  double altitude=salt(sd, st, latitude);
  altitudeParam=altitude;
  double azimuth = sazi(sd, st, latitude)+north;

  // make sure azimuth is in [-PI,PI]
  if(azimuth > M_PI) {
    azimuth -= 2*M_PI;
  } else if(azimuth < -M_PI) {
    azimuth += 2*M_PI;
  }

  if(altitude > 87.*M_PI/180.) {
    fprintf(stderr,"warning - sun too close to zenith, reducing altitude to 87 degrees\n");
    altitude = 87.*M_PI/180.;
  }

  polar2vec(sundir, altitude, azimuth);
  sundir=-1*sundir;
  //printf("solar alt,azi:%f,%f\n", altitude, azimuth);
  //printf("solar position:%f,%f,%f\n",sundir.x,sundir.y,sundir.z);
  //printf("latitude %f longitude %f \n", latitude, longitude);


}

int jdate(		/* Julian date (days into year) */
	int month,
	int day
)
{
	static short  mo_da[12] = {0,31,59,90,120,151,181,212,243,273,304,334};

	return(mo_da[month-1] + day);
}

double getSkyDistrFactor(float3& v, float altitude)
{// only clear sky has distribution factor
   double distrFactor;

  const float a = -1.0;
  const float b = -0.32;
  const float c = 10;
  const float d = -3.0;
  const float e = 0.45;
  //if(skyType == SViewConfig::eCIE_Clear_Sky_Turbid){
  //  a=a2, b=b2, c=c2, d=d2;

  //getSunVector(v);
  distrFactor = ((a+b*exp(-3*(M_PI/2-altitude))+c*v.y*v.y)*(1-exp(-d)));

  //printf("F2:%f alt: %f\n", distrFactor, altitude);
  return distrFactor;
}

float getSunNormalizedIllum(float3 sundir)
{


  // use solar birghtness algorithm from Radiance
  double sunbri =  1.5e9/SUNEFFICACY * (1.147 - .147/(sundir.y>.16?sundir.y:.16));
  //calcSolarBrightness();
  static const double sunSolidAngle = sin(0.25*M_PI/180)*sin(0.25*M_PI/180);//*M_PI;
  // calculate the irradiance from the sun
  float sunIrradiance = static_cast<float> (sunbri*sunSolidAngle);
  // we divide the sun brightness by PI because light are idealy reflected by the reflector in the
  // whole hemisphere, so the radiance from a reflector should be the irradiance/PI, but opengl
  // does not do the division, so we do it here instead.
  // NOW WE DO IT BY OMITTING *M_PI IN THE CALCULATION OF sunSolidAngle
  // sunIrradiance /= M_PI;
  //std::cout<<"sunIrradiance:"<<sunIrradiance<<",sunSolidAngle:"<<sunSolidAngle<<std::endl;
  return sunIrradiance;

}
