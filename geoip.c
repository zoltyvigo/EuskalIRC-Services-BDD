/* Servicio GeoIP, botshispanobdd
 *
 * (C) 2009 donostiarra  admin.euskalirc@gmail.com  http://euskalirc.wordpress.com
 *
 * Este programa es software libre. Puede redistribuirlo y/o modificarlo
 * bajo los términos de la Licencia Pública General de GNU según es publicada
 * por la Free Software Foundation.
 */
#include "services.h"
#include "pseudo.h"
 /* Main Geoip routine. */

void redirec(char **av)  {
int i,longit;

/*normalizo los dominios y los paso a minúsculas de la tabla de códigos de país de GeoIP localización*/ 
const char *GeoIP_dominio_codigo[258] = { "NET","ORG","COM","EDU","EUS","CAT",".ap",".eu",".ad",".ae",".af",".ag",".ai",".al",".am",".an]", /*16*/
	".ao",".aq",".ar",".as",".at",".au",".aw",".az",".ba",".bb", /*10*/
	".bd",".be]",".bf",".bg",".bh",".bi",".bj",".bm",".bn",".bo",
	".br",".bs",".bt",".bv",".bw",".by",".bz",".ca",".cc",".cd",
	".cf",".cg",".ch",".ci",".ck",".cl",".cm",".cn",".co",".cr",
	".cu",".cv",".cx",".cy",".cz",".de",".dj",".dk",".dm",".do",  /*hay 24 filas de 10 dominios = 24x10=240*/
	".dz",".ec",".ee",".eg",".eh",".er",".es",".et",".fi",".fj",
	".fk",".fm",".fo",".fr",".fx",".ga",".gb",".gd",".ge",".gf",
	".gh",".gi",".gl",".gm",".gn",".gp",".gq",".gr",".gs",".gt",
	".gu",".gw",".gy",".hk",".hm",".hn",".hr",".ht",".hu",".id",
	".ie",".il",".in",".io",".iq",".ir",".is",".it",".jm",".jo",
	".jp",".ke",".kg",".kh",".ki",".km",".kn",".kp",".kr",".kw",
	".KY",".KZ",".LA",".LB",".LC",".LI",".LK",".LR",".LS",".LT",
	".LU",".LV",".LY",".MA",".MC",".MD",".MG",".MH",".MK",".ML",
	".MM",".MN",".MO",".MP",".MQ",".MR",".MS",".MT",".MU",".MV",
	".MW",".MX",".MY",".MZ",".NA",".NC",".NE",".NF",".NG",".NI",
	".NL",".NO",".NP",".NR",".NU",".NZ",".OM",".PA",".PE",".PF",
	".PG",".PH",".PK",".PL",".PM",".PN",".PR",".PS",".PT",".PW",
	".PY",".QA",".RE",".RO",".RU",".RW",".SA",".SB",".SC",".SD",
	".SE",".SG",".SH",".SI",".SJ",".SK",".SL",".SM",".SN",".SO",
	".SR",".ST",".SV",".SY",".SZ",".TC",".TD",".TF",".TG",".TH",
	".TJ",".tk",".TM",".TN",".TO",".TL",".TR",".TT",".TV",".TW",
	".TZ",".UA",".UG",".UM",".US",".UY",".UZ",".VA",".VC",".VE",
	".VG",".VI",".VN",".VU",".WF",".WS",".YE",".YT",".RS",".ZA",
	".ZM",".ME",".ZW",".A1",".A2",".O1",".AX",".GG",".IM",".JE",
  ".BL",".MF"}; /*2*/

/*los paises asociados a canales*/

const char * GeoIP_canal[258] = {CanalCybers,CanalCybers,CanalCybers,CanalCybers,"Euskadi","Catalunya","Asia/Pacific/Region","Europe","Andorra","United_Arab_Emirates","Afghanistan","Antigua_and_Barbuda","Anguilla","Albania","Armenia","Netherlands_Antilles",
	"Angola","Antarctica","Argentina","American Samoa","Austria","Australia","Aruba","Azerbaijan","Bosnia_and_Herzegovina","Barbados",
	"Bangladesh","Belgium","Burkina_Faso","Bulgaria","Bahrain","Burundi","Benin","Bermuda","Brunei_Darussalam","Bolivia",
	"Brazil","Bahamas","Bhutan","Bouvet_Island","Botswana","Belarus","Belize","Canada","Cocos_(Keeling)_Islands","Congo",
	"Central_African_Republic","Congo","Switzerland","Cote_D'Ivoire","Cook_Islands","Chile","Cameroon","China","Colombia","Costa Rica",
	"Cuba","Cape_Verde","Christmas_Island","Cyprus","Czech_Republic","Germany","Djibouti","Denmark","Dominica","Dominican_Republic",
	"Algeria","Ecuador","Estonia","Egypt","Western_Sahara","Eritrea","Spain","Ethiopia","Finland","Fiji",
	"Malvinas","Micronesia","Faroe_Islands","France","France(Metropolitan)","Gabon","United_Kingdom","Grenada","Georgia","French_Guiana",
	"Ghana","Gibraltar","Greenland","Gambia","Guinea","Guadeloupe","Equatorial_Guinea","Greece","South_Georgia&South_Sandwich_Islands","Guatemala",
	"Guam","Guinea-Bissau","Guyana","Hong Kong","Heard_Island&McDonald_Islands","Honduras","Croatia","Haiti","Hungary","Indonesia",
	"Ireland","Israel","India","British_Indian_Ocean_Territory","Iraq","Iran","Iceland","Italy","Jamaica","Jordan",
	"Japan","Kenya","Kyrgyzstan","Cambodia","Kiribati","Comoros","Saint_Kitts& Nevis","Korea_Democratic_Republic","Korea_Republic","Kuwait",
	"Cayman_Islands","Kazakhstan","Lao_Democratic_Republic","Lebanon","Saint_Lucia","Liechtenstein","Sri_Lanka","Liberia","Lesotho","Lithuania",
	"Luxembourg","Latvia","Libyan_Arab_Jamahiriya","Morocco","Monaco","Moldova","Madagascar","Marshall_Islands","Macedonia","Mali",
	"Myanmar","Mongolia","Macau","Northern_Mariana_Islands","Martinique","Mauritania","Montserrat","Malta","Mauritius","Maldives",
	"Malawi","Mexico","Malaysia","Mozambique","Namibia","New_Caledonia","Niger","Norfolk_Island","Nigeria","Nicaragua",
	"Netherlands","Norway","Nepal","Nauru","Niue","New_Zealand","Oman","Panama","Peru","French_Polynesia",
	"Papua_New_Guinea","Philippines","Pakistan","Poland","Saint_Pierre&Miquelon","Pitcairn_Islands","Puerto_Rico","Palestinian_Territory","Portugal","Palau",
	"Paraguay","Qatar","Reunion","Romania","Russian_Federation","Rwanda","Saudi_Arabia","Solomon_Islands","Seychelles","Sudan",
	"Sweden","Singapore","Saint_Helena","Slovenia","Svalbard&Jan_Mayen","Slovakia","Sierra_Leone","San_Marino","Senegal","Somalia","Suriname",
	"Sao_Tome&Principe","El_Salvador","Syrian_Arab_Republic","Swaziland","Turks&Caicos_Islands","Chad","French_Southern_Territories","Togo","Thailand",
	"Tajikistan","Tokelau","Turkmenistan","Tunisia","Tonga","Timor-Leste","Turkey","Trinidad&Tobago","Tuvalu","Taiwan",
	"Tanzania_United_Republic","Ukraine","Uganda","United_States_Minor_Outlying_Islands","United_States","Uruguay","Uzbekistan","Holy_See_(Vatican_City_State)","Saint_Vincent&the_Grenadines","Venezuela",
	"Virgin_Islands_British","Virgin_Islands_U.S.","Vietnam","Vanuatu","Wallis&Futuna","Samoa","Yemen","Mayotte","Serbia","South Africa",
	"Zambia","Montenegro","Zimbabwe","Anonymous_Proxy","Satellite_Provider","Other","Aland_Islands","Guernsey","Isle_of_Man","Jersey",
  "Saint_Barthelemy","Saint_Martin"};

   /*calculo longitud porque sólo me interesa comparar el dominio que es el final del host*/
     longit=strlen(av[4]);

       char *cad=av[4];
       char cadena[10];

       cadena[0] = cad[longit-3];
       cadena[1] = cad[longit-2];
       cadena[2] = cad[longit-1];
       cadena[3] = '\0';

    
	  

 for (i=0; i<=252 ; i++) {
         
    /*comparo las 2 cadenas ignorando  mayúsculas  y  minúsculas*/

       if ( strcasecmp(cadena,GeoIP_dominio_codigo[i])== 0) {
      send_cmd(ServerName, "SVSJOIN  %s #%s", av[0],GeoIP_canal[i]);
      canaladmins(s_GeoIP, "2ENTRA: %s 12HOST[%s]5(%s) 12CANAL:4 #%s",av[0],av[4],cadena,GeoIP_canal[i]);
      return;
      }
    
   }

}

void do_geoip(User *u) /*la colocamos en extern.h y asi la llamamos desde oper*/
{    
        char *cmd;
        cmd = strtok(NULL, " ");
       
      if ((!cmd) || ((!stricmp(cmd, "ON") == 0) && (!stricmp(cmd, "OFF") == 0)&& (!stricmp(cmd, "STATUS") == 0))) {
      	 notice_lang(s_GeoIP, u, GEOIP_HELP);
    	return;
    }
   
      if  ((stricmp(cmd, "ON") == 0)) {
	if (!is_services_oper(u)) {
	    notice_lang(s_GeoIP, u, PERMISSION_DENIED);
	    return;
	} 
       autogeoip=1;
         canaladmins(s_GeoIP, "2ACTIVADO por 12%s",u->nick);
	  privmsg(s_GeoIP, u->nick, "Servicio GeoIP 2ACTIVADO");
        }

     if ((stricmp(cmd, "OFF") == 0))  {
	if (!is_services_oper(u)) {
	    notice_lang(s_GeoIP, u, PERMISSION_DENIED);
	    return;
	} 
	autogeoip=0;
           canaladmins(s_GeoIP, "4DESACTIVADO por 12%s",u->nick);
	 privmsg(s_GeoIP, u->nick, "Servicio GeoIP 5DESACTIVADO");
	
         }
if ((stricmp(cmd, "STATUS") == 0))  {
	if (!is_services_oper(u)) {
	    notice_lang(s_GeoIP, u, PERMISSION_DENIED);
	    return;
	} 
	if (autogeoip) 
             privmsg(s_GeoIP, u->nick, "Estado Servicio GeoIP 2ACTIVADO");
         else
               privmsg(s_GeoIP, u->nick, "Estado Servicio GeoIP 5DESACTIVADO");
               
}

}




