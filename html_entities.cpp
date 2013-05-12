/*
======================================================================

RSS Power Tool Source Code
Copyright (C) 2013 Gregory Naughton

This file is part of RSS Power Tool

RSS Power Tool is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RSS Power Tool is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RSS Power Tool  If not, see <http://www.gnu.org/licenses/>.

======================================================================
*/


// from: HTML entities print reference -- http://www.html-entities.org/print.php
//
// best possible html entity translation into ascii replacement
//

#include <string.h>
#include "html_entities.h"

struct html_entities_s
{
    const char * display;
    const char * entity;
    const char * literal;
} 
__html_entities[] = {

{ "\"", "&#34;", "&quot;" },     //  quotation mark
{ "'",  "&#39;", "&apos;" },  //  apostrophe
{ "&",  "&#38;", "&amp;" },   //   ampersand
{ "<",  "&#60;", "&lt;" },    //    less-than
{ ">",  "&#62;", "&gt;" },    //    greater-than

/* ISO 8859-1 symbols */

{ " ",              "&#160;",   "&nbsp;" },     //  non-breaking space
{ "!",              "&#161;",   "&iexcl;" },    // inverted exclamation mark
{ "c",              "&#162;",   "&cent;" },     //  cent
{ "British Pound",  "&#163;",   "&pound;" },    // pound
{ "c",              "&#164;",   "&curren;" },   //    currency
{ "Yen",            "&#165;",   "&yen;" },      //   yen
{ "|",              "&#166;",   "&brvbar;" },   //    broken vertical bar
{ "sect.",          "&#167;",   "&sect;" },     //  section
{ "\"",             "&#168;",   "&uml;" },      //   spacing diaeresis
{ "(c)",            "&#169;",   "&copy;" },     //  copyright
{ "a",              "&#170;",   "&ordf;" },     //  feminine ordinal indicator
{ "<<",             "&#171;",   "&laquo;" },    // angle quotation mark (left)
{ "~",              "&#172;",   "&not;" },      //   negation
{ "-",              "&#173;",   "&shy;" },      //   soft hyphen
{ "(r)",            "&#174;",   "&reg;" },      //   registered trademark
{ "-",              "&#175;",   "&macr;" },     //  spacing macron
{ "degree",         "&#176;",   "&deg;" },      //   degree
{ "+/-",            "&#177;",   "&plusmn;" },   //    plus-or-minus
{ "2",              "&#178;",   "&sup2;" },     //  superscript 2
{ "3",              "&#179;",   "&sup3;" },     //  superscript 3
{ "'",              "&#180;",   "&acute;" },    // spacing acute
{ "u",              "&#181;",   "&micro;" },    // micro
{ "PP",             "&#182;",   "&para;" },     //  paragraph
{ ".",              "&#183;",   "&middot;" },   //    middle dot
{ ",",              "&#184;",   "&cedil;" },    // spacing cedilla
{ "1",              "&#185;",   "&sup1;" },     //  superscript 1
{ "o",              "&#186;",   "&ordm;" },     //  masculine ordinal indicator
{ ">>",             "&#187;",   "&raquo;" },    // angle quotation mark (right)
{ "1/4",            "&#188;",   "&frac14;" },   //    fraction 1/4
{ "1/2",            "&#189;",   "&frac12;" },   //    fraction 1/2
{ "1/3",            "&#190;",   "&frac34;" },   //    fraction 3/4
{ "?",              "&#191;",   "&iquest;" },   //    inverted question mark
{ "*",              "&#215;",   "&times;" },    // multiplication
{ "/",              "&#247;",   "&divide;" },   //    division

/* ISO 8859-1 characters */

{"A",   "&#192;",   "&Agrave;" },       // capital a, grave accent
{"A",   "&#193;",   "&Aacute;" },       // capital a, acute accent
{"A",    "&#194;",  "&Acirc;" },        // capital a, circumflex accent
{"A",    "&#195;",  "&Atilde;" },       // capital a, tilde
{"A",    "&#196;",  "&Auml;" },     // capital a, umlaut mark
{"A",    "&#197;",  "&Aring;" },        // capital a, ring
{ "AE",   "&#198;", "&AElig;" },        // capital ae
{ "C",   "&#199;",  "&Ccedil;" },       // capital c, cedilla
{ "E",   "&#200;",  "&Egrave;" },       // capital e, grave accent
{ "E",   "&#201;",  "&Eacute;" },       // capital e, acute accent
{ "E",   "&#202;",  "&Ecirc;" },        // capital e, circumflex accent
{ "E",   "&#203;",  "&Euml;" },     // capital e, umlaut mark
{ "I",   "&#204;",  "&Igrave;" },       // capital i, grave accent
{ "I",   "&#205;",  "&Iacute;" },       // capital i, acute accent
{ "I",   "&#206;",  "&Icirc;" },        // capital i, circumflex accent
{ "I",   "&#207;",  "&Iuml;" },     // capital i, umlaut mark
{ "D",   "&#208;",  "&ETH;" },      // capital eth, Icelandic
{ "N",   "&#209;",  "&Ntilde;" },       // capital n, tilde
{ "O",   "&#210;",  "&Ograve;" },       // capital o, grave accent
{ "O",   "&#211;",  "&Oacute;" },       // capital o, acute accent
{ "O",   "&#212;",  "&Ocirc;" },        // capital o, circumflex accent
{ "O",   "&#213;",  "&Otilde;" },       // capital o, tilde
{ "O",   "&#214;",  "&Ouml;" },     // capital o, umlaut mark
{ "0",   "&#216;",  "&Oslash;" },       // capital o, slash
{ "U",   "&#217;",  "&Ugrave;" },       // capital u, grave accent
{ "U",   "&#218;",  "&Uacute;" },       // capital u, acute accent
{ "U",   "&#219;",  "&Ucirc;" },        // capital u, circumflex accent
{ "U",   "&#220;",  "&Uuml;" },     // capital u, umlaut mark
{ "Y",   "&#221;",  "&Yacute;" },       // capital y, acute accent
{ "P",   "&#222;",  "&THORN;" },        // capital THORN, Icelandic
{ "B",   "&#223;",  "&szlig;" },        // small sharp s, German
{ "a",   "&#224;",  "&agrave;" },       // small a, grave accent
{ "a",   "&#225;",  "&aacute;" },       // small a, acute accent
{ "a",   "&#226;",  "&acirc;" },        // small a, circumflex accent
{ "a",   "&#227;",  "&atilde;" },       // small a, tilde
{ "a",   "&#228;",  "&auml;" },     // small a, umlaut mark
{ "a",   "&#229;",  "&aring;" },        // small a, ring
{ "ae",   "&#230;", "&aelig;" },        // small ae
{ "c",   "&#231;",  "&ccedil;" },       // small c, cedilla
{ "e",   "&#232;",  "&egrave;" },       // small e, grave accent
{ "e",   "&#233;",  "&eacute;" },       // small e, acute accent
{ "e",   "&#234;",  "&ecirc;" },        // small e, circumflex accent
{ "e",   "&#235;",  "&euml;" },     // small e, umlaut mark
{ "i",   "&#236;",  "&igrave;" },       // small i, grave accent
{ "i",   "&#237;",  "&iacute;" },       // small i, acute accent
{ "i",   "&#238;",  "&icirc;" },        // small i, circumflex accent
{ "i",   "&#239;",  "&iuml;" },     // small i, umlaut mark
{ "o",   "&#240;",  "&eth;" },      // small eth, Icelandic
{ "n",   "&#241;",  "&ntilde;" },       // small n, tilde
{ "o",   "&#242;",  "&ograve;" },       // small o, grave accent
{ "o",   "&#243;",  "&oacute;" },       // small o, acute accent
{ "o",   "&#244;",  "&ocirc;" },        // small o, circumflex accent
{ "o",   "&#245;",  "&otilde;" },       // small o, tilde
{ "o",   "&#246;",  "&ouml;" },     // small o, umlaut mark
{ "0",   "&#248;",  "&oslash;" },       // small o, slash
{ "u",   "&#249;",  "&ugrave;" },       // small u, grave accent
{ "u",   "&#250;",  "&uacute;" },       // small u, acute accent
{ "u",   "&#251;",  "&ucirc;" },        // small u, circumflex accent
{ "u",   "&#252;",  "&uuml;" },     // small u, umlaut mark
{ "y",   "&#253;",  "&yacute;" },       // small y, acute accent
{ "p",   "&#254;",  "&thorn;" },        // small thorn, Icelandic
{ "y",   "&#255;",  "&yuml;" },     // small y, umlaut mark

/* math symbols */

{ "A",      "&#8704;",  "&forall;" },      //   for all
{ "p",      "&#8706;",  "&part;" },        // part
{ "3",      "&#8707;",  "&exist;" },       //exists
{ "o",      "&#8709;",  "&empty;" },        // empty
{ "V",      "&#8711;",  "&nabla;" },        // nabla
{ "E",      "&#8712;",  "&isin;" },     // isin
{ "E",      "&#8713;",  "&notin;" },        // notin
{ "3",      "&#8715;",  "&ni;" },       // ni
{ "prod",   "&#8719;",  "&prod;" },     // prod
{ "E",      "&#8721;",  "&sum;" },      // sum
{ "-",      "&#8722;",  "&minus;" },        // minus
{ "*",      "&#8727;",  "&lowast;" },       // lowast
{ "sqrt(",  "&#8730;",  "&radic;" },    // square root
{ "~",      "&#8733;",  "&prop;" },     // proportional to
{ "inf",    "&#8734;",  "&infin;" },    // infinity
{ "ang",    "&#8736;",  "&ang;" },  // angle
{ "^",      "&#8743;",  "&and;" },  // and
{ "v",      "&#8744;",  "&or;" },   // or
{ "^",      "&#8745;",  "&cap;" },  // cap
{ "U",      "&#8746;",  "&cup;" },  // cup
{ "int(",   "&#8747;",  "&int;" },  // integral
{ "therefore","&#8756;",    "&there4;" },   // therefore
{ "~",      "&#8764;",  "&sim;" },  // similar to
{ "~=",     "&#8773;",  "&cong;" },     // congruent to
{ "~=",     "&#8776;",  "&asymp;" },    // almost equal
{ "!=",     "&#8800;",  "&ne;" },   // not equal
{ "==",     "&#8801;",  "&equiv;" },    // equivalent
{ "<=",     "&#8804;",  "&le;" },   // less or equal
{ ">=",     "&#8805;",  "&ge;" },   // greater or equal
{ "subset", "&#8834;",  "&sub;" },  // subset of
{ "supset", "&#8835;",  "&sup;" },  // superset of
{ "!subset","&#8836;",  "&nsub;" },     // not subset of
{ "subset=","&#8838;",  "&sube;" },     // subset or equal
{ "supset=","&#8839;",  "&supe;" },     // superset or equal
{ "(+)",    "&#8853;",  "&oplus;" },    // circled plus
{ "(x)",    "&#8855;",  "&otimes;" },   // circled times
{ "_|_",    "&#8869;",  "&perp;" },     // perpendicular
{ "dot",    "&#8901;",  "&sdot;" },     //  dot operator

/* Greek letters */

{ "A",  "&#913;",   "&Alpha;" },    // Alpha
{ "B",  "&#914;",   "&Beta;" },    // Beta
{ "G",  "&#915;",   "&Gamma;" },   //Gamma
{ "D",  "&#916;",   "&Delta;" },   //Delta
{ "E",  "&#917;",   "&Epsilon;" }, //  Epsilon
{ "Z",  "&#918;",   "&Zeta;" },    // Zeta
{ "H",  "&#919;",   "&Eta;" },      // Eta
{ "O",  "&#920;",   "&Theta;" },    // Theta
{ "I",  "&#921;",   "&Iota;" }, // Iota
{ "K",  "&#922;",   "&Kappa;" },    // Kappa
{ "L",  "&#923;",   "&Lambda;" },   // Lambda
{ "M",  "&#924;",   "&Mu;" },   // Mu
{ "N",  "&#925;",   "&Nu;" },   // Nu
{ "X",  "&#926;",   "&Xi;" },   // Xi
{ "O",  "&#927;",   "&Omicron;" },  // Omicron
{ "P",  "&#928;",   "&Pi;" },   // Pi
{ "P",  "&#929;",   "&Rho;" },  // Rho
{ "S",  "&#207;",   "&sigmaf;" },   // Sigmaf
{ "E",  "&#931;",   "&Sigma;" },    // Sigma
{ "T",  "&#932;",   "&Tau;" },  // Tau
{ "Y",  "&#933;",   "&Upsilon;" },  // Upsilon
{ "Phi",    "&#934;",   "&Phi;" },  // Phi
{ "X",  "&#935;",   "&Chi;" },  // Chi
{ "Psi",    "&#936;",   "&Psi;" },  // Psi
{ "Om", "&#937;",   "&Omega;" },    // Omega
{ "a",  "&#945;",   "&alpha;" },    // alpha
{ "B",  "&#946;",   "&beta;" }, // beta
{ "Y",  "&#947;",   "&gamma;" },    // gamma
{ "d",  "&#948;",   "&delta;" },    // delta
{ "e",  "&#949;",   "&epsilon;" },  // epsilon
{ "z",  "&#950;",   "&zeta;" }, // zeta
{ "n",  "&#951;",   "&eta;" },  // eta
{ "O",  "&#952;",   "&theta;" },    // theta
{ "i",  "&#953;",   "&iota;" }, // iota
{ "k",  "&#954;",   "&kappa;" },    // kappa
{ "l",  "&#955;",   "&lambda;" },   // lambda
{ "mu", "&#956;",   "&mu;" },   // mu
{ "nu", "&#957;",   "&nu;" },   // nu
{ "E",  "&#958;",   "&xi;" },   // xi
{ "o",  "&#959;",   "&omicron;" },  // omicron
{ "pi", "&#960;",   "&pi;" },   // pi
{ "rho",    "&#961;",   "&rho;" },  // rho
{ "c",  "&#962;",   "&sigmaf;" },   // sigmaf
{ "sigma",  "&#963;",   "&sigma;" },    // sigma
{ "t",  "&#964;",   "&tau;" },  // tau
{ "v",  "&#965;",   "&upsilon;" },  // upsilon
{ "phi",    "&#966;",   "&phi;" },  // phi
{ "X",  "&#967;",   "&chi;" },  // chi
{ "psi",    "&#968;",   "&psi;" },  // psi
{ "omega",  "&#969;",   "&omega;" },    // omega
{ "theta",  "&#977;",   "&thetasym;" }, // theta symbol
{ "Y",  "&#978;",   "&upsih;" },    // upsilon symbol
{ "w",  "&#982;",   "&piv;" },  // pi symbol

/* other entities */

{ "OE",     "&#338;",   "&OElig;" },    // capital ligature OE
{ "oe",     "&#339;",   "&oelig;" },    // small ligature oe
{ "S",      "&#352;",   "&Scaron;" },   // capital S with caron
{ "s",      "&#353;",   "&scaron;" },   // small S with caron
{ "Y",      "&#376;",   "&Yuml;" }, // capital Y with diaeres
{ "f",      "&#402;",   "&fnof;" }, // f with hook
{ "^",      "&#710;",   "&circ;" }, // modifier letter circumflex accent
{ "~",      "&#732;",   "&tilde;" },    // small tilde
{ " ",      "&#8194;",  "&ensp;" }, // en space
{ " ",      "&#8195;",  "&emsp;" }, // em space
{ " ",      "&#8201;",  "&thinsp;" },   // thin space
{ "0",      "&#8204;",  "&zwnj;" }, // zero width non-joiner
{ "0-",     "&#8205;",  "&zwj;" },  // zero width joiner
{ "-->",    "&#8206;",  "&lrm;" },  // left-to-right mark
{ "<--",    "&#8207;",  "&rlm;" },  // right-to-left mark
{ "--",     "&#8211;",  "&ndash;" },    // en dash
{ "--",     "&#8212;",  "&mdash;" },    // em dash
{ "'",      "&#8216;",  "&lsquo;" },    // left single quotation mark
{ "'",      "&#8217;",  "&rsquo;" },    // right single quotation mark
{ "'",      "&#8218;",  "&sbquo;" },    // single low-9 quotation mark
{ "\"",     "&#8220;",  "&ldquo;" },    // left double quotation mark
{ "\"",     "&#8221;",  "&rdquo;" },    // right double quotation mark
{ "\"",     "&#8222;",  "&bdquo;" },    // double low-9 quotation mark
{ "t",      "&#8224;",  "&dagger;" },   // dagger
{ "tt",     "&#8225;",  "&Dagger;" },   // double dagger
{ "o",      "&#8226;",  "&bull;" }, // bullet
{ "...",    "&#8230;",  "&hellip;" },   // horizontal ellipsis
{ "o",      "&#8240;",  "&permil;" },   // per mille
{ "'",      "&#8242;",  "&prime;" },    // minutes
{ "\"",     "&#8243;",  "&Prime;" },    // seconds
{ "<",      "&#8249;",  "&lsaquo;" },   // single left angle quotation
{ ">",      "&#8250;",  "&rsaquo;" },   // single right angle quotation
{ "_",      "&#8254;",  "&oline;" },    // overline
{ "euro",   "&#8364;",  "&euro;" }, // euro
{ "tm",     "&#8482;",  "&trade;" },    // trademark
{ "<=",     "&#8592;",  "&larr;" }, // left arrow
{ "-^",     "&#8593;",  "&uarr;" }, // up arrow
{ "=>",     "&#8594;",  "&rarr;" }, // right arrow
{ "v-",     "&#8595;",  "&darr;" }, // down arrow
{ "<=>",    "&#8596;",  "&harr;" }, // left right arrow
{ "<-+",    "&#8629;",  "&crarr;" },    // carriage return arrow
{ "I",      "&#8968;",  "&lceil;" },    // left ceiling
{ "I",      "&#8969;",  "&rceil;" },    // right ceiling
{ "I",      "&#8970;",  "&lfloor;" },   // left floor
{ "I",      "&#8971;",  "&rfloor;" },   // right floor
{ "V",      "&#9674;",  "&loz;" },  // lozenge
{ "spade",  "&#9824;",  "&spades;" },   // spade
{ "club",   "&#9827;",  "&clubs;" },    // club
{ "<3",     "&#9829;",  "&hearts;" },   // heart
{ "diamond","&#9830;",  "&diams;" },    // diamond
{ "/",      "&#8260;",  "&frasl;" },    // fraction slash
{ "P",      "&#8472;",  "&weierp;" },   // script capital P = power set = Weierstrass p
{ "I",      "&#8465;",  "&image;" },    // black-letter capital I = imaginary part
{ "R",      "&#8476;",  "&real;" }, // black-letter capital R = real part symbol
{ "alef",   "&#8501;",  "&alefsym;" },  // alef symbol = first transfinite cardinal
{ "<-",     "&#8656;",  "&lArr;" }, // leftwards double arrow
{ "^-",     "&#8657;",  "&uArr;" }, // upwards double arrow
{ "->",     "&#8658;",  "&rArr;" }, // rightwards double arrow
{ "-v",     "&#8659;",  "&dArr;" }, // downwards double arrow
{ "<-->",   "&#8660;",  "&hArr;" }, // left right double arrow
{ "{",      "&#9001;",  "&lang;" }, // left-pointing angle bracket = bra
{ "}",      "&#9002;",  "&rang;" }, // right-pointing angle bracket = ket
{ 0, 0, 0 } };

struct uri_s
{
    const char match;
    const char * replace; 
}
__uri_components[] = {
{ ' ', "%20" },
{ '!', "%21" },
{ '"', "%22" },
{ '#', "%23" },
{ '$', "%24" },
{ '%', "%25" },
{ '&', "%26" },
{ '\'', "%27" },
{ '(', "%28" },
{ ')', "%29" },
{ '*', "%2A" },
{ '+', "%2B" },
{ ',', "%2C" },
{ '-', "%2D" },
{ '.', "%2E" },
{ '/', "%2F" },
{ '0', "%30" },
{ '1', "%31" },
{ '2', "%32" },
{ '3', "%33" },
{ '4', "%34" },
{ '5', "%35" },
{ '6', "%36" },
{ '7', "%37" },
{ '8', "%38" },
{ '9', "%39" },
{ ':', "%3A" },
{ ';', "%3B" },
{ '<', "%3C" },
{ '=', "%3D" },
{ '>', "%3E" },
{ '?', "%3F" },
{ '@', "%40" },
{ 'A', "%41" },
{ 'B', "%42" },
{ 'C', "%43" },
{ 'D', "%44" },
{ 'E', "%45" },
{ 'F', "%46" },
{ 'G', "%47" },
{ 'H', "%48" },
{ 'I', "%49" },
{ 'J', "%4A" },
{ 'K', "%4B" },
{ 'L', "%4C" },
{ 'M', "%4D" },
{ 'N', "%4E" },
{ 'O', "%4F" },
{ 'P', "%50" },
{ 'Q', "%51" },
{ 'R', "%52" },
{ 'S', "%53" },
{ 'T', "%54" },
{ 'U', "%55" },
{ 'V', "%56" },
{ 'W', "%57" },
{ 'X', "%58" },
{ 'Y', "%59" },
{ 'Z', "%5A" },
{ '[', "%5B" },
{ '\\', "%5C" },
{ ']', "%5D" },
{ '^', "%5E" },
{ '_', "%5F" },
{ '`', "%60" },
{ 'a', "%61" },
{ 'b', "%62" },
{ 'c', "%63" },
{ 'd', "%64" },
{ 'e', "%65" },
{ 'f', "%66" },
{ 'g', "%67" },
{ 'h', "%68" },
{ 'i', "%69" },
{ 'j', "%6A" },
{ 'k', "%6B" },
{ 'l', "%6C" },
{ 'm', "%6D" },
{ 'n', "%6E" },
{ 'o', "%6F" },
{ 'p', "%70" },
{ 'q', "%71" },
{ 'r', "%72" },
{ 's', "%73" },
{ 't', "%74" },
{ 'u', "%75" },
{ 'v', "%76" },
{ 'w', "%77" },
{ 'x', "%78" },
{ 'y', "%79" },
{ 'z', "%7A" },
{ '{', "%7B" },
{ '|', "%7C" },
{ '}', "%7D" },
{ '~', "%7E" },
{ 0, 0 } };


const char * HtmlEntities_t::swap_uri( const char find )
{
    struct uri_s * p = __uri_components;
    do
    {
        if ( find == p->match )
            return p->replace;
    } 
    while ( (++p)->match );
    return "";
}


const char * HtmlEntities_t::swap_numeric( const char * test ) 
{
    if ( !test || !*test )
        return 0;
    struct html_entities_s * p = __html_entities;
    do
    {
        if ( strcmp( test, p->entity ) == 0 )
            return p->display;
    }
    while ( (++p)->display );
    return unknown_ent;
}

const char * HtmlEntities_t::swap_literal( const char * test ) 
{
    if ( !test || !*test )
        return 0;
    struct html_entities_s * p = __html_entities;
    do
    {
        if ( strcmp( test, p->literal ) == 0 )
            return p->display;
    }
    while ( (++p)->display );
    return unknown_ent; // might be another one not in the list?
}

unsigned int HtmlEntities_t::longest_entity()
{
    unsigned int e_len = 0;
    struct html_entities_s *p = __html_entities;
    do
    {
        if ( strlen( p->entity ) > e_len )
            e_len = strlen( p->entity );
    }
    while ( (++p)->display );
    return e_len;
}

unsigned int HtmlEntities_t::longest_literal()
{
    unsigned int l_len = 0;
    struct html_entities_s *p = __html_entities;
    do
    {
        if ( strlen( p->literal ) > l_len )
            l_len = strlen( p->literal );
    }
    while ( (++p)->display );
    return l_len;
}

HtmlEntities_t::HtmlEntities_t() : unknown_ent(" ")
{ }



