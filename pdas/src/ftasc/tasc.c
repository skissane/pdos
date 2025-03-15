/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards                             */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  This program takes an integer as a parameter.  The integer       */
/*  should have a representation of a character.  An integer         */
/*  is returned, containing the representation of that character     */
/*  in the ASCII character set.                                      */
/*                                                                   */
/*********************************************************************/
int tasc(int loc)
{
  switch (loc)
  {
    case '\t' : return (0x09);
    case '\n' : return (0x0a);
    case '\f' : return (0x0c);
    case '\r' : return (0x0d);
    case ' '  : return (0x20);
    case '!'  : return (0x21);
    case '\"' : return (0x22);
    case '#'  : return (0x23);
    case '$'  : return (0x24);
    case '%'  : return (0x25);
    case '&'  : return (0x26);
    case '\'' : return (0x27);
    case '('  : return (0x28);
    case ')'  : return (0x29);
    case '*'  : return (0x2a);
    case '+'  : return (0x2b);
    case ','  : return (0x2c);
    case '-'  : return (0x2d);
    case '.'  : return (0x2e);
    case '/'  : return (0x2f);
    case '0'  : return (0x30);
    case '1'  : return (0x31);
    case '2'  : return (0x32);
    case '3'  : return (0x33);
    case '4'  : return (0x34);
    case '5'  : return (0x35);
    case '6'  : return (0x36);
    case '7'  : return (0x37);
    case '8'  : return (0x38);
    case '9'  : return (0x39);
    case ':'  : return (0x3a);
    case ';'  : return (0x3b);
    case '<'  : return (0x3c);
    case '='  : return (0x3d);
    case '>'  : return (0x3e);
    case '?'  : return (0x3f);
    case '@'  : return (0x40);
    case 'A'  : return (0x41);
    case 'B'  : return (0x42);
    case 'C'  : return (0x43);
    case 'D'  : return (0x44);
    case 'E'  : return (0x45);
    case 'F'  : return (0x46);
    case 'G'  : return (0x47);
    case 'H'  : return (0x48);
    case 'I'  : return (0x49);
    case 'J'  : return (0x4a);
    case 'K'  : return (0x4b);
    case 'L'  : return (0x4c);
    case 'M'  : return (0x4d);
    case 'N'  : return (0x4e);
    case 'O'  : return (0x4f);
    case 'P'  : return (0x50);
    case 'Q'  : return (0x51);
    case 'R'  : return (0x52);
    case 'S'  : return (0x53);
    case 'T'  : return (0x54);
    case 'U'  : return (0x55);
    case 'V'  : return (0x56);
    case 'W'  : return (0x57);
    case 'X'  : return (0x58);
    case 'Y'  : return (0x59);
    case 'Z'  : return (0x5a);
    case '['  : return (0x5b);
    case '\\' : return (0x5c);
    case ']'  : return (0x5d);
    case '^'  : return (0x5e);
    case '_'  : return (0x5f);
    case '`'  : return (0x60);
    case 'a'  : return (0x61);
    case 'b'  : return (0x62);
    case 'c'  : return (0x63);
    case 'd'  : return (0x64);
    case 'e'  : return (0x65);
    case 'f'  : return (0x66);
    case 'g'  : return (0x67);
    case 'h'  : return (0x68);
    case 'i'  : return (0x69);
    case 'j'  : return (0x6a);
    case 'k'  : return (0x6b);
    case 'l'  : return (0x6c);
    case 'm'  : return (0x6d);
    case 'n'  : return (0x6e);
    case 'o'  : return (0x6f);
    case 'p'  : return (0x70);
    case 'q'  : return (0x71);
    case 'r'  : return (0x72);
    case 's'  : return (0x73);
    case 't'  : return (0x74);
    case 'u'  : return (0x75);
    case 'v'  : return (0x76);
    case 'w'  : return (0x77);
    case 'x'  : return (0x78);
    case 'y'  : return (0x79);
    case 'z'  : return (0x7a);
    case '{'  : return (0x7b);
    case '|'  : return (0x7c);
    case '}'  : return (0x7d);
    case '~'  : return (0x7e);
    default   : return(0);
  }
}
