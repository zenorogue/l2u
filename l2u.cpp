// Latex to Unicode
// Copyright (C) 2011-2019 Zeno Rogue

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

// convert the standard input from LaTeX to Unicode

#include <stdio.h>
#include <map>
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;

template<class T> int isize(const T& x) { return x.size(); }

string buffer;

/** @brief read the next char from stdin */
void readmore() {
  int g = getchar();
  if(g < 0) return;
  buffer += char(g);
  }

/** @brief did stdin end? */
bool eof() {
  if(buffer.empty()) readmore();
  return buffer.empty();
  }

/** @brief next character on stdin */
char next() {
  if(buffer.empty()) readmore();
  return buffer[0];
  }

/** @brief consume s characters from stdin */
void advance(int s = 1) {
  while(isize(buffer) < s)  readmore();
  buffer = buffer.substr(s);
  }

/** @brief return the next character on stdin, removing it */
char getnext() {
  if(eof()) return 'x';
  char c = next();
  advance();
  return c;
  }

/** @brief if the next characters in stdin equal s, return true and consume them */
bool eat(const string& s) {
  for(int i=0; i<isize(s); i++) {
    if(isize(buffer) <= i) readmore();
    if(buffer[i] != s[i]) {
      return false;
      }
    }
  advance(isize(s));
  return true;
  }

/** @brief translate the codepoint to an UTF8 string
 *  source: http://www.zedwood.com/article/cpp-utf8-char-to-codepoint 
 */
string utf8chr(unsigned cp)
{
    char c[5]={ 0x00,0x00,0x00,0x00,0x00 };
    if     (cp<=0x7F) { c[0] = cp;  }
    else if(cp<=0x7FF) { c[0] = (cp>>6)+192; c[1] = (cp&63)+128; }
    else if(0xd800<=cp && cp<=0xdfff) {} //invalid block of utf8
    else if(cp<=0xFFFF) { c[0] = (cp>>12)+224; c[1]= ((cp>>6)&63)+128; c[2]=(cp&63)+128; }
    else if(cp<=0x10FFFF) { c[0] = (cp>>18)+240; c[1] = ((cp>>12)&63)+128; c[2] = ((cp>>6)&63)+128; c[3]=(cp&63)+128; }
    return string(c);
}

/** @brief cut off the next UTF8 character from s, starting at pos; move pos to the next UTF8 character */
string utf8cut(const string& s, int& pos) {
  int take;
  unsigned char u = s[pos];
  if(u < 128) take = 1;
  else if(u < 224) take = 2;
  else if(u < 240) take = 3;
  else take = 4;
  string res = s.substr(pos, take);
  pos += take;
  return res;
  }

/** a translation of characters */
struct mapping {
  /** LaTeX command to do this */
  string command;
  /** original characters, in UTF8 */
  string origs;
  /** modified characters, in UTF8 */
  string modified;
  };

/** a mapping that does nothing */
mapping nullmapping {"nul", "", ""};

mapping mathit{"\\mathit", "ΑΒΧΔΕΗΓΙΚΛΜΝΩΟΦΠΨΡΣΤΘΥΞΖαβχδεηγικλμνωοφπψρστθυξζabcdefgijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", "𝛢𝛣𝛸𝛥𝛦𝛨𝛤𝛪𝛫𝛬𝛭𝛮𝛺𝛰𝛷𝛱𝛹𝛲𝛴𝛵𝛩𝛶𝛯𝛧𝛼𝛽𝜒𝛿𝜀𝜂𝛾𝜄𝜅𝜆𝜇𝜈𝜔𝜊𝜑𝜋𝜓𝜌𝜎𝜏𝜃𝜐𝜉𝜁𝑎𝑏𝑐𝑑𝑒𝑓𝑔𝑖𝑗𝑘𝑙𝑚𝑛𝑜𝑝𝑞𝑟𝑠𝑡𝑢𝑣𝑤𝑥𝑦𝑧𝐴𝐵𝐶𝐷𝐸𝐹𝐺𝐻𝐼𝐽𝐾𝐿𝑀𝑁𝑂𝑃𝑄𝑅𝑆𝑇𝑈𝑉𝑊𝑋𝑌𝑍"};

vector<mapping> mappings = {
  mapping{"_", "0123456789+-=()aehijklmnoprstuvx?", "₀₁₂₃₄₅₆₇₈₉₊₋₌₍₎ₐₑₕᵢⱼₖₗₘₙₒₚᵣₛₜᵤᵥₓₔ"},
  mapping{"^", "0123456789+-=()niabcdefghijklmnoprstuvwxyz", "⁰¹²³⁴⁵⁶⁷⁸⁹⁺⁻⁼⁽⁾ⁿⁱᵃᵇᶜᵈᵉᶠᵍʰⁱʲᵏˡᵐⁿᵒᵖʳˢᵗᵘᵛʷˣʸᶻ"},
  mapping{"\\bb", "abcdefghijklmnopqrstuwvxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789", "𝕒𝕓𝕔𝕕𝕖𝕗𝕘𝕙𝕚𝕛𝕜𝕝𝕞𝕟𝕠𝕡𝕢𝕣𝕤𝕥𝕦𝕧𝕨𝕩𝕪𝕫𝔸𝔹ℂ𝔻𝔼𝔽𝔾ℍ𝕀𝕁𝕂𝕃𝕄ℕ𝕆ℙℚℝ𝕊𝕋𝕌𝕍𝕎𝕏𝕐ℤ𝟘𝟙𝟚𝟛𝟜𝟝𝟞𝟟𝟠𝟡"},
  mapping{"\\mathbb", "abcdefghijklmnopqrstuwvxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789", "𝕒𝕓𝕔𝕕𝕖𝕗𝕘𝕙𝕚𝕛𝕜𝕝𝕞𝕟𝕠𝕡𝕢𝕣𝕤𝕥𝕦𝕧𝕨𝕩𝕪𝕫𝔸𝔹ℂ𝔻𝔼𝔽𝔾ℍ𝕀𝕁𝕂𝕃𝕄ℕ𝕆ℙℚℝ𝕊𝕋𝕌𝕍𝕎𝕏𝕐ℤ𝟘𝟙𝟚𝟛𝟜𝟝𝟞𝟟𝟠𝟡"},
  mapping{"\\'", "AÆCEGIKLMNOPRSUWYZaæcegiklmnoprsuwyz", "ÁǼĆÉǴÍḰĹḾŃÓṔŔŚÚẂÝŹáǽćéǵíḱĺḿńóṕŕśúẃýź"},
  mapping{"\\`", "ИЕиеAEINOUWYaeinouwy", "ЍЀѝѐÀÈÌǸÒÙẀỲàèìǹòùẁỳ"},
  mapping{"\\\"", "ͣͦͧАӨЧЭИОӘУЫЗЖаөчэиоәуызж‐AEHIOUWXYaehiotuwxy", "ᷲᷳᷴӒӪӴӬӤӦӚӰӸӞӜӓӫӵӭӥӧӛӱӹӟӝ⸚ÄËḦÏÖÜẄẌŸäëḧïöẗüẅẍÿ"},
  mapping{"\\^", "ACEGHIJOSUWYZaceghijosuwyz", "ÂĈÊĜĤÎĴÔŜÛŴŶẐâĉêĝĥîĵôŝûŵŷẑ"},
  mathit,
  mapping{"\\it", "ΑΒΧΔΕΗΓΙΚΛΜΝΩΟΦΠΨΡΣΤΘΥΞΖαβχδεηγικλμνωοφπψρστθυξζabcdefgijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", "𝛢𝛣𝛸𝛥𝛦𝛨𝛤𝛪𝛫𝛬𝛭𝛮𝛺𝛰𝛷𝛱𝛹𝛲𝛴𝛵𝛩𝛶𝛯𝛧𝛼𝛽𝜒𝛿𝜀𝜂𝛾𝜄𝜅𝜆𝜇𝜈𝜔𝜊𝜑𝜋𝜓𝜌𝜎𝜏𝜃𝜐𝜉𝜁𝑎𝑏𝑐𝑑𝑒𝑓𝑔𝑖𝑗𝑘𝑙𝑚𝑛𝑜𝑝𝑞𝑟𝑠𝑡𝑢𝑣𝑤𝑥𝑦𝑧𝐴𝐵𝐶𝐷𝐸𝐹𝐺𝐻𝐼𝐽𝐾𝐿𝑀𝑁𝑂𝑃𝑄𝑅𝑆𝑇𝑈𝑉𝑊𝑋𝑌𝑍"},
  mapping{"\\mathbf", "ΑΒΧΔΕΗΓΙΚΛΜΝΩΟΦΠΨΡΣΤΘΥΞΖαβχδϝεηγικλμνωοφπψρστθυξζabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", "𝚨𝚩𝚾𝚫𝚬𝚮𝚪𝚰𝚱𝚲𝚳𝚴𝛀𝚶𝚽𝚷𝚿𝚸𝚺𝚻𝚯𝚼𝚵𝚭𝛂𝛃𝛘𝛅𝟋𝛆𝛈𝛄𝛊𝛋𝛌𝛍𝛎𝛚𝛐𝛗𝛑𝛙𝛒𝛔𝛕𝛉𝛖𝛏𝛇𝐚𝐛𝐜𝐝𝐞𝐟𝐠𝐡𝐢𝐣𝐤𝐥𝐦𝐧𝐨𝐩𝐪𝐫𝐬𝐭𝐮𝐯𝐰𝐱𝐲𝐳𝐀𝐁𝐂𝐃𝐄𝐅𝐆𝐇𝐈𝐉𝐊𝐋𝐌𝐍𝐎𝐏𝐐𝐑𝐒𝐓𝐔𝐕𝐖𝐗𝐘𝐙"},
  mapping{"\\bf", "ΑΒΧΔΕΗΓΙΚΛΜΝΩΟΦΠΨΡΣΤΘΥΞΖαβχδϝεηγικλμνωοφπψρστθυξζabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", "𝚨𝚩𝚾𝚫𝚬𝚮𝚪𝚰𝚱𝚲𝚳𝚴𝛀𝚶𝚽𝚷𝚿𝚸𝚺𝚻𝚯𝚼𝚵𝚭𝛂𝛃𝛘𝛅𝟋𝛆𝛈𝛄𝛊𝛋𝛌𝛍𝛎𝛚𝛐𝛗𝛑𝛙𝛒𝛔𝛕𝛉𝛖𝛏𝛇𝐚𝐛𝐜𝐝𝐞𝐟𝐠𝐡𝐢𝐣𝐤𝐥𝐦𝐧𝐨𝐩𝐪𝐫𝐬𝐭𝐮𝐯𝐰𝐱𝐲𝐳𝐀𝐁𝐂𝐃𝐄𝐅𝐆𝐇𝐈𝐉𝐊𝐋𝐌𝐍𝐎𝐏𝐐𝐑𝐒𝐓𝐔𝐕𝐖𝐗𝐘𝐙"},
  mapping{"\\mathcal", "abcdfhijklmnpqrstuvwxyzACDGJKNOPQSTUVWXYZ", "𝒶𝒷𝒸𝒹𝒻𝒽𝒾𝒿𝓀𝓁𝓂𝓃𝓅𝓆𝓇𝓈𝓉𝓊𝓋𝓌𝓍𝓎𝓏𝒜𝒞𝒟𝒢𝒥𝒦𝒩𝒪𝒫𝒬𝒮𝒯𝒰𝒱𝒲𝒳𝒴𝒵"},
  mapping{"\\cal", "abcdfhijklmnpqrstuvwxyzACDGJKNOPQSTUVWXYZ", "𝒶𝒷𝒸𝒹𝒻𝒽𝒾𝒿𝓀𝓁𝓂𝓃𝓅𝓆𝓇𝓈𝓉𝓊𝓋𝓌𝓍𝓎𝓏𝒜𝒞𝒟𝒢𝒥𝒦𝒩𝒪𝒫𝒬𝒮𝒯𝒰𝒱𝒲𝒳𝒴𝒵"}
  };

/** a named character */
struct named {
  /** what character it is */
  string text;
  /** LaTeX command creating this character */
  string command;
  };

vector<named> nameds = {
  {"\\", "\\\\"},
  {"{", "\\{"},
  {"}", "\\}"},
  {"$", "\\$"},
{"Α", "\\Alpha"},
{"Β", "\\Beta"},
{"Χ", "\\Chi"},
{"Δ", "\\Delta"},
{"Ε", "\\Epsilon"},
{"Η", "\\Eta"},
{"Γ", "\\Gamma"},
{"Ͱ", "\\Heta"},
{"Ι", "\\Iota"},
{"Κ", "\\Kappa"},
{"Λ", "\\Lambda"},
{"Μ", "\\Mu"},
{"Ν", "\\Nu"},
{"Ω", "\\Omega"},
{"Ο", "\\Omicron"},
{"Φ", "\\Phi"},
{"Π", "\\Pi"},
{"Ψ", "\\Psi"},
{"Ρ", "\\Rho"},
{"Ϻ", "\\San"},
{"Ϸ", "\\Sho"},
{"Σ", "\\Sigma"},
{"Τ", "\\Tau"},
{"Θ", "\\Theta"},
{"Υ", "\\Upsilon"},
{"Ξ", "\\Xi"},
{"Ϳ", "\\Yot"},
{"Ζ", "\\Zeta"},
{"α", "\\alpha"},
{"β", "\\beta"},
{"χ", "\\chi"},
{"δ", "\\delta"},
{"ϝ", "\\digamma"},
{"ε", "\\epsilon"},
{"η", "\\eta"},
{"γ", "\\gamma"},
{"ͱ", "\\heta"},
{"ι", "\\iota"},
{"κ", "\\kappa"},
{"ϟ", "\\koppa"},
{"λ", "\\lambda"},
{"μ", "\\mu"},
{"ν", "\\nu"},
{"ω", "\\omega"},
{"ο", "\\omicron"},
{"φ", "\\phi"},
{"π", "\\pi"},
{"ψ", "\\psi"},
{"ρ", "\\rho"},
{"ϡ", "\\sampi"},
{"ϻ", "\\san"},
{"ϸ", "\\sho"},
{"σ", "\\sigma"},
{"ϛ", "\\stigma"},
{"τ", "\\tau"},
{"θ", "\\theta"},
{"υ", "\\upsilon"},
{"ξ", "\\xi"},
{"ζ", "\\zeta"},
{"∞", "\\infty"},
{"∫", "\\int"},
{"∈", "\\in"},
{"∋", "\\ni"},
{"∉", "\\notin"},
{"∌", "\\notni"},
{"×", "\\times"},
{"¬", "\\neg"},
{"∧", "\\wedge"},
{"∨", "\\vee"},
{"≡", "\\equiv"},
{"≢", "\\nequiv"},
{"∃", "\\exists"},
{"∀", "\\forall"},
{"√", "\\sqrt"},
{"∅", "\\emptyset"},
{"⟶", "\\ra"},
{"⟶", "\\rightarrow"},
{"≠", "\\neq"},
{"≤", "\\leq"},
{"≥", "\\geq"},
{"°", "^\\circ"},
  };

void print_mapping(const string& command, mapping& m) {
  printf("mapping{\"%s\", \"%s\", \"%s\"}\n", command.c_str(), m.origs.c_str(), m.modified.c_str());
  }

/** @brief generate a mapping, based on the UNICODE NamesList.txt read into @arg m */
void generate_mapping(map<string, string>& m, string command, string suffix) {
  mapping ma;
  for(auto p: m)
    if(m.count(p.first + suffix))
      ma.origs += m[p.first],
      ma.modified += m[p.first+suffix];
  print_mapping(command, ma);
  }

/** @brief tolower string except the first letter */
string be_Like(string s) {
  for(int i=1; i<isize(s); i++)
    s[i] = tolower(s[i]);
  return s;
  }

/** @brief tolower string */
string be_like(string s) {
  for(int i=0; i<isize(s); i++)
    s[i] = tolower(s[i]);
  return s;
  }


/** @brief a function to generate the mappings and named tables, based on the NamesList.txt file */
void generate_mappings() {
  map<string, string> m;
  ifstream fs("NamesList.txt");
  if(!fs.is_open()) {
    system("wget https://unicode.org/Public/UNIDATA/NamesList.txt");
    fs.open("NamesList.txt");
    }
  string s;
  while(getline(fs, s)) {
    if(!isalnum(s[0])) continue;
    int p = s.find("\t");
    if(p != string::npos) {
      string codepoint = s.substr(0, p);
      string name = s.substr(p+1);
      unsigned cp = strtoll(codepoint.c_str(), nullptr, 16);
      string u = utf8chr(cp);;
      m[name] = u;
      // cout << name << " = " << m[name] << "\n";
      }
    }
  
  generate_mapping(m, "\\'", " WITH ACUTE");
  generate_mapping(m, "\\`", " WITH GRAVE");
  generate_mapping(m, "\\\"", " WITH DIAERESIS");
  generate_mapping(m, "\\^", " WITH CIRCUMFLEX");
  
  // Greek letters -- note that lambda is called lamda for some reason
  
  mapping mitalic, mbold, mcal;
  
  auto addif = [&] (string ch, string name, mapping& ma) {
    if(m.count(name)) {
      printf("mapping %s to %s\n", ch.c_str(), name.c_str());
      ma.origs += ch;
      ma.modified += m[name];
      return true;
      }
    printf("not mapping %s to %s\n", ch.c_str(), name.c_str());
    return false;
    };
  
  for(auto p: m) {
    string name = p.first;
    if(name.substr(0, 21) == "GREEK CAPITAL LETTER " && name.find(" ", 21) == string::npos) {
      string lname = name.substr(21);
      printf("{\"%s\", \"\\%s\"},\n", p.second.c_str(), be_Like(lname).c_str());
      
      addif(p.second, "MATHEMATICAL ITALIC CAPITAL " + lname, mitalic);
      addif(p.second, "MATHEMATICAL BOLD CAPITAL " + lname, mbold);
      }
    if(name.substr(0, 19) == "GREEK SMALL LETTER " && name.find(" ", 19) == string::npos) {
      string lname = name.substr(19);
      printf("{\"%s\", \"\\%s\"},\n", p.second.c_str(), be_like(lname).c_str());
      addif(p.second, "MATHEMATICAL ITALIC SMALL " + lname, mitalic);
      addif(p.second, "MATHEMATICAL BOLD SMALL " + lname, mbold);
      }
    }
  
  for(bool cap: {false, true}) for(char ch='a'; ch <= 'z'; ch++) {
    string name = cap ? "CAPITAL " : "SMALL ";
    name += char(ch - 32);
    char actual = (cap ? char(ch - 32) : ch);
    string ach = ""; ach += actual;
    addif(ach, "MATHEMATICAL ITALIC " + name, mitalic);
    addif(ach, "MATHEMATICAL BOLD " + name, mbold);
    addif(ach, "MATHEMATICAL SCRIPT " + name, mcal);
    }
  
  print_mapping("\\\\mathit", mitalic);
  print_mapping("\\\\it", mitalic);
  print_mapping("\\\\mathbf", mbold);
  print_mapping("\\\\bf", mbold);
  print_mapping("\\\\mathcal", mcal);
  print_mapping("\\\\cal", mcal);
  }

/** @brief apply a mapping to an UTF8 character, output to stdout */
void apply_mapping(mapping& m, string ch) {
  int p1 = 0, p2 = 0;
  while(p1 < isize(m.origs)) {
    string c1 = utf8cut(m.origs, p1);
    string c2 = utf8cut(m.modified, p2);
    if(c1 == ch) { cout << c2; return; }
    }
  cout << ch;
  }

/** @brief apply a mapping (or nullmapping) to a character or LaTeX block {...} on stdin */
void modify_one(mapping& m) {
  if(eof()) {
    cout << "(error:" << m.command << ")";
    return;
    }
  
  if(eat("{")) {
    while(!eat("}")) 
      modify_one(m);
    return;
    }

  if(eat("$") && &m == &nullmapping) {
    while(!eat("$"))
      modify_one(mathit);
    return;
    }
  
  for(auto& na: nameds)
    if(eat(na.command)) {
      apply_mapping(m, na.text);
      return;
      }
  
  for(auto& ma: mappings)
    if(eat(ma.command)) {
      modify_one(ma);
      return;
      }
    
  string s0 = "";
  s0 += getnext();
  apply_mapping(m, s0);
  }

int main(int argc, char **argv) {

  if(argc>1 && string(argv[1]) == "-g") {
    generate_mappings();
    exit(1);
    }
  
  while(!eof()) {
    modify_one(nullmapping);
    }

  return 0;
  }
