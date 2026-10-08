#include "App.h"
#include "MathEngine.h"
#include "Physics.h"
#include "Chemistry.h"
#include "SDManager.h"
#include "WifiManager.h"
#include <M5Cardputer.h>
#include <SD.h>
#include <math.h>

namespace {
const int W=240,H=135;
enum Screen { HOME,MATH,PHYSICS,CHEMISTRY,PERIODIC,CHEM_INFO,BONDS,BIOLOGY,FILES,WIFI,NOTES,SD_BROWSER,SD_TEXT,IMAGE_VIEW,CALC,EQUATION,EXPONENTIAL,TRIG,GRAPH,PHYSICS_RUN,NOTES_EDIT };
Screen screen=HOME;
int selected=0;
bool dirty=true;
String input="";
String message="";
String path="/";
String textCache="";
int textOffset=0;
uint8_t chemZ=1,chemB=8;
uint8_t physTarget=0;
bool graphEditing=false;
double graphCenter=0,graphScale=1;

const char* homeName[]={"MATEMATICA","FISICA","QUIMICA","TABELA PERIODICA","BIOLOGIA","ARQUIVOS SD","WI-FI","ANOTACOES"};
const char* homeIcon[]={"Σ","F","Q","118","BIO","SD","Wi","✎"};
const char* mathName[]={"CALCULADORA","RESOLVER EQUACAO","EXPONENCIAL","TRIG / LOG / RAIZ","GRAFICO / FUNCAO","NOTAS","ARQUIVOS"};
const char* mathIcon[]={"π","=","aˣ","sin","ƒ(x)","N","SD"};
const char* physName[]={"ASSISTENTE","CINEMATICA","FORCA","ENERGIA","DENSIDADE","TRABALHO","NOTAS"};
const char* physIcon[]={"?","v","F","E","ρ","W","N"};
const char* chemName[]={"TABELA 118","ELEMENTO","LIGACOES","MATERIAIS"};
const char* chemIcon[]={"118","⚛","⇄","SD"};

void bg(){M5Cardputer.Display.fillScreen(TFT_BLACK);M5Cardputer.Display.setTextSize(1);}
void txt(const String&s,int x,int y,uint16_t c=TFT_WHITE){M5Cardputer.Display.setTextColor(c);M5Cardputer.Display.drawString(s,x,y);}
void header(const String&s){txt(s,5,3,TFT_CYAN);M5Cardputer.Display.drawFastHLine(4,16,232,TFT_DARKGREY);}
void footer(const String&s="← → navegar   ENTER abrir   DEL voltar"){txt(s,4,124,TFT_DARKGREY);}
void card(int x,int y,int w,int h,const String&ic,const String&name,bool sel,uint16_t accent=TFT_CYAN){if(sel){M5Cardputer.Display.fillRoundRect(x,y,w,h,4,accent);txt(ic,x+5,y+7,TFT_BLACK);txt(name,x+5,y+h-14,TFT_BLACK);}else{M5Cardputer.Display.drawRoundRect(x,y,w,h,4,TFT_DARKGREY);txt(ic,x+5,y+7,accent);txt(name,x+5,y+h-14,TFT_WHITE);}}
void horizontal(const char**names,const char**icons,int n,const String&t){bg();header(t);int start=selected-1;if(start<0)start=0;if(start>n-3)start=max(0,n-3);for(int i=0;i<3&&start+i<n;i++){int idx=start+i;card(5+i*78,25,72,72,icons[idx],names[idx],idx==selected);}txt(String(selected+1)+" / "+String(n),205,7,TFT_DARKGREY);footer();}
void drawHome(){bg();header("CENTRAL DE ESTUDOS");int n=8,start=selected-1;if(start<0)start=0;if(start>n-3)start=n-3;for(int i=0;i<3;i++){int idx=start+i;card(5+i*78,26,72,73,homeIcon[idx],homeName[idx],idx==selected,(idx==0?TFT_CYAN:idx==1?TFT_YELLOW:idx==2?TFT_YELLOW:idx==3?TFT_GREEN:idx==4?TFT_MAGENTA:idx==5?TFT_BLUE:idx==6?TFT_CYAN:TFT_MAGENTA));}txt(SDManager::ready()?"SD conectado":"SD nao detectado",5,108,SDManager::ready()?TFT_GREEN:TFT_RED);footer("← → mudar area   ENTER abrir");}
void drawMath(){horizontal(mathName,mathIcon,7,"MATEMATICA");}
void drawPhys(){horizontal(physName,physIcon,7,"FISICA");}
void drawChem(){horizontal(chemName,chemIcon,4,"QUIMICA");}
void drawInput(const String&t,const String&hint){bg();header(t);txt(hint,5,23,TFT_DARKGREY);M5Cardputer.Display.drawRoundRect(4,39,232,29,4,TFT_DARKGREY);String shown=input;if(shown.length()>38)shown=shown.substring(shown.length()-38);txt(shown,8,49,TFT_WHITE);if(message.length()){int p=0;for(int i=0;i<3&&p<message.length();i++){int e=message.indexOf('\n',p);if(e<0)e=message.length();String s=message.substring(p,e);if(s.length()>39)s=s.substring(0,39);txt(s,5,76+i*13,TFT_YELLOW);p=e+1;}}footer("ENTER calcular   DEL voltar");}
void drawCalc(){drawInput("CALCULADORA","Digite como numa calculadora: 2+3*4   2^5   sqrt(25)   sin(pi/2)");}
void drawEquation(const String&t){drawInput(t,"Digite a equacao inteira; o sistema identifica o tipo sozinho.");}
uint16_t categoryColor(const String&c){String s=c;s.toLowerCase();if(s.indexOf("gas nobre")>=0)return TFT_BLUE;if(s.indexOf("halogenio")>=0)return TFT_MAGENTA;if(s.indexOf("alcalino")>=0)return TFT_RED;if(s.indexOf("lantan")>=0||s.indexOf("actin")>=0)return TFT_YELLOW;if(s.indexOf("metal")>=0)return TFT_YELLOW;if(s.indexOf("semimetal")>=0)return TFT_GREEN;return TFT_CYAN;}
int zAt(int per,int grp){for(size_t i=0;i<Chemistry::count();i++){const auto&e=Chemistry::all()[i];if(atoi(e.period)==per&&atoi(e.group)==grp)return e.z;}return 0;}
void periodicCell(int z,int x,int y,int cw,int ch,bool sel){if(!z)return;const auto&e=*Chemistry::byAtomicNumber(z);uint16_t col=categoryColor(e.category);if(sel){M5Cardputer.Display.fillRect(x,y,cw-1,ch-1,col);txt(e.symbol,x+1,y+2,TFT_BLACK);}else{M5Cardputer.Display.drawRect(x,y,cw-1,ch-1,col);txt(e.symbol,x+1,y+2,col);}}
void drawPeriodic(){bg();header("TABELA PERIODICA • 118 ELEMENTOS");int cw=12,ch=11,x0=6,y0=21;for(int p=1;p<=7;p++)for(int g=1;g<=18;g++){int z=zAt(p,g);periodicCell(z,x0+(g-1)*cw,y0+(p-1)*ch,cw,ch,z==chemZ);}for(int i=0;i<15;i++){int z=57+i;periodicCell(z,57+i*12,101,12,10,z==chemZ);int a=89+i;periodicCell(a,57+i*12,113,12,10,a==chemZ);}txt("La-Lu",5,103,TFT_DARKGREY);txt("Ac-Lr",5,115,TFT_DARKGREY);txt("Cor = familia   ENTER = detalhes",5,124,TFT_DARKGREY);}
void drawChemInfo(){const auto*e=Chemistry::byAtomicNumber(chemZ);bg();header("ELEMENTO • DETALHE");if(!e){txt("Elemento invalido",5,30);return;}txt(String(e->symbol)+"  "+e->name,5,25,TFT_CYAN);txt("Z "+String(e->z)+"   grupo "+e->group+"   periodo "+e->period,5,40);txt(String(e->category),5,54,categoryColor(e->category));txt("Camadas: "+String(e->config),5,68,TFT_WHITE);txt(Chemistry::octetNote(*e),5,82,TFT_YELLOW);txt("←/→ outro elemento   ENTER ligacoes",5,101,TFT_DARKGREY);footer("DEL voltar");}
void drawBonds(){const auto*a=Chemistry::byAtomicNumber(chemZ),*b=Chemistry::byAtomicNumber(chemB);bg();header("LIGACAO QUIMICA");txt("A  "+String(a->symbol)+"  "+a->name,5,26,TFT_CYAN);txt("B  "+String(b->symbol)+"  "+b->name,5,43,TFT_CYAN);txt("TIPO",5,62,TFT_DARKGREY);txt(Chemistry::bondType(*a,*b),45,60,TFT_GREEN);txt("A: "+String(a->config),5,79);txt("B: "+String(b->config),5,94);txt("←/→ muda A   ↑/↓ muda B",5,110,TFT_DARKGREY);}
void drawPhysRun(){bg();header(String("FISICA • ")+Physics::name(physTarget));txt("Formula sugerida:",5,24,TFT_DARKGREY);txt(Physics::formula(physTarget),5,38,TFT_CYAN);txt("Informe somente os dados que possui:",5,52,TFT_DARKGREY);M5Cardputer.Display.drawRect(5,61,230,23,TFT_DARKGREY);txt(input,9,68);int p=0;for(int i=0;i<2&&p<message.length();i++){int e=message.indexOf('\n',p);if(e<0)e=message.length();txt(message.substring(p,e),5,91+i*13,TFT_YELLOW);p=e+1;}footer("ENTER calcular   DEL voltar");}
void drawGraph(){bg();header("GRAFICO • y = "+input);int ox=120,oy=78;M5Cardputer.Display.drawFastHLine(5,oy,230,TFT_DARKGREY);M5Cardputer.Display.drawFastVLine(ox,22,96,TFT_DARKGREY);int px=-1,py=-1;for(int sx=5;sx<235;sx+=2){double x=(sx-ox)/(18.0*graphScale)+graphCenter;auto r=MathEngine::evaluate(input,x);if(!r.ok||fabs(r.value)>12){px=-1;continue;}int sy=oy-(int)(r.value*7*graphScale);if(sy<20||sy>120){px=-1;continue;}if(px>=0)M5Cardputer.Display.drawLine(px,py,sx,sy,TFT_GREEN);px=sx;py=sy;}txt(graphEditing?"EDITANDO EXPRESSAO":"←/→ desloca  ;/. zoom  ENTER edita",5,121,TFT_DARKGREY);}
void drawSD(){bg();header("SD • "+path);String l=SDManager::list(path);int p=0,line=0;while(line<7&&p<l.length()){int e=l.indexOf('\n',p);if(e<0)e=l.length();String s=l.substring(p,e);if(line==selected)M5Cardputer.Display.fillRect(2,22+line*14,236,13,TFT_CYAN);txt(s,5,24+line*14,line==selected?TFT_BLACK:TFT_WHITE);p=e+1;line++;}footer("ENTER abrir   ← volta pasta   ↑↓ selecionar");}
void drawText(){bg();header(textCache.substring(0,25));int p=textOffset;for(int i=0;i<7&&p<textCache.length();i++){int e=textCache.indexOf('\n',p);if(e<0)e=textCache.length();String s=textCache.substring(p,e);if(s.length()>39)s=s.substring(0,39);txt(s,3,22+i*14);p=e+1;}footer("↑/↓ rolar   DEL voltar");}
String extension(const String&p){int i=p.lastIndexOf('.');if(i<0)return "";String e=p.substring(i+1);e.toLowerCase();return e;}
void drawImage(){
 bg();
 header("IMAGEM • SD");
 String ext=extension(path);
 if(ext=="jpg"||ext=="jpeg") M5Cardputer.Display.drawJpgFile(SD,path.c_str(),0,18,240,103);
 else if(ext=="png") M5Cardputer.Display.drawPngFile(SD,path.c_str(),0,18,240,103);
 else if(ext=="bmp") M5Cardputer.Display.drawBmpFile(SD,path.c_str(),0,18);
 else txt("Formato de imagem nao suportado",5,40,TFT_RED);
 footer("DEL voltar");
}
void drawNotesMenu(){bg();header("ANOTACOES");card(5,27,72,72,"+","NOVA",selected==0,TFT_GREEN);card(83,27,72,72,"SD","LER",selected==1,TFT_CYAN);card(161,27,72,72,"N","MATERIAIS",selected==2,TFT_YELLOW);footer();}
void draw(){if(!dirty)return;dirty=false;switch(screen){case HOME:drawHome();break;case MATH:drawMath();break;case PHYSICS:drawPhys();break;case CHEMISTRY:drawChem();break;case PERIODIC:drawPeriodic();break;case CHEM_INFO:drawChemInfo();break;case BONDS:drawBonds();break;case BIOLOGY:path="/BIOLOGIA";drawSD();break;case FILES:drawSD();break;case WIFI:bg();header("WI-FI • TRANSFERENCIA");txt("Rede: Cardputer-Estudos",5,30,TFT_CYAN);txt("Endereco: 192.168.4.1",5,47);txt("Envie arquivos pelo navegador",5,64);txt("Arquivos ficam no SD",5,81,TFT_GREEN);footer("DEL voltar");break;case NOTES:drawNotesMenu();break;case SD_BROWSER:drawSD();break;case SD_TEXT:drawText();break;case IMAGE_VIEW:drawImage();break;case CALC:drawCalc();break;case EQUATION:drawEquation("RESOLVER EQUACAO");break;case EXPONENTIAL:drawEquation("EQUACAO EXPONENCIAL");break;case TRIG:drawInput("TRIG / LOG / RAIZ","Ex.: sen(pi/2)   log(100)   sqrt(25)");break;case GRAPH:drawGraph();break;case PHYSICS_RUN:drawPhysRun();break;case NOTES_EDIT:drawInput("NOVA ANOTACAO","Digite e ENTER salva no SD.");break;}}
void goHome(){screen=HOME;selected=0;input="";message="";path="/";dirty=true;}
void back(){if(screen==HOME)return;if(screen==SD_TEXT||screen==IMAGE_VIEW){screen=SD_BROWSER;dirty=true;return;}if(screen==SD_BROWSER){goHome();return;}if(screen==PERIODIC||screen==CHEM_INFO||screen==BONDS){screen=CHEMISTRY;selected=0;dirty=true;return;}if(screen==CALC||screen==EQUATION||screen==EXPONENTIAL||screen==TRIG||screen==GRAPH){screen=MATH;selected=0;input="";message="";dirty=true;return;}if(screen==PHYSICS_RUN){screen=PHYSICS;selected=0;dirty=true;return;}if(screen==NOTES_EDIT){screen=NOTES;dirty=true;return;}goHome();}
void appendInput(char c){if(c=='`')return;if(c==',')c='.';if(c>=32&&c<=126){input+=c;dirty=true;}}
void activate(){
 if(screen==HOME){switch(selected){case 0:screen=MATH;break;case 1:screen=PHYSICS;break;case 2:screen=CHEMISTRY;break;case 3:screen=PERIODIC;chemZ=1;break;case 4:screen=BIOLOGY;path="/BIOLOGIA";break;case 5:screen=FILES;path="/";break;case 6:screen=WIFI;break;case 7:screen=NOTES;break;}selected=0;input="";message="";dirty=true;return;}
 if(screen==MATH){switch(selected){case 0:screen=CALC;break;case 1:screen=EQUATION;break;case 2:screen=EXPONENTIAL;break;case 3:screen=TRIG;break;case 4:screen=GRAPH;input="x^2";graphEditing=false;break;case 5:screen=SD_BROWSER;path="/MATEMATICA";break;case 6:screen=SD_BROWSER;path="/MATEMATICA";break;}selected=0;input=screen==GRAPH?input:"";message="";dirty=true;return;}
 if(screen==PHYSICS){if(selected==6){screen=SD_BROWSER;path="/FISICA";}else{physTarget=selected;screen=PHYSICS_RUN;}input="";message="";dirty=true;return;}
 if(screen==CHEMISTRY){if(selected==0)screen=PERIODIC;else if(selected==1)screen=CHEM_INFO;else if(selected==2)screen=BONDS;else {screen=SD_BROWSER;path="/QUIMICA";}selected=0;dirty=true;return;}
 if(screen==PERIODIC){screen=CHEM_INFO;dirty=true;return;}
 if(screen==CHEM_INFO){screen=BONDS;dirty=true;return;}
 if(screen==CALC||screen==TRIG){auto r=MathEngine::evaluate(input);message=r.text;dirty=true;return;}
 if(screen==EQUATION){auto r=MathEngine::solveEquation(input);message=r.text;dirty=true;return;}
 if(screen==EXPONENTIAL){auto r=MathEngine::solveExponential(input);message=r.text;dirty=true;return;}
 if(screen==GRAPH){graphEditing=!graphEditing;dirty=true;return;}
 if(screen==PHYSICS_RUN){message=Physics::solve(physTarget,input);dirty=true;return;}
 if(screen==NOTES){if(selected==0){screen=NOTES_EDIT;input="";message="";}else if(selected==1){screen=SD_BROWSER;path="/ANOTACOES";selected=0;}else{screen=SD_BROWSER;path="/MATERIAIS";selected=0;}dirty=true;return;}
 if(screen==BONDS){chemB=(chemB>=118)?1:chemB+1;dirty=true;return;}
 if(screen==NOTES_EDIT){if(input.length()){String p="/ANOTACOES/nota_"+String(millis())+".txt";message=SDManager::writeText(p,input)?"Anotacao salva no SD":"Falha ao salvar";}dirty=true;return;}
 if(screen==SD_BROWSER||screen==BIOLOGY||screen==FILES){String l=SDManager::list(path);int p=0,line=0;while(p<l.length()){int e=l.indexOf('\n',p);if(e<0)e=l.length();if(line==selected){String s=l.substring(p,e);if(s.startsWith("[D] ")){path=SDManager::normalize(path,s.substring(4));selected=0;}else if(s.startsWith("[F] ")){String fp=SDManager::normalize(path,s.substring(4));String ext=extension(fp);if(ext=="txt"||ext=="md"||ext=="csv"||ext=="log"||ext=="ini"){textCache=SDManager::readText(fp);textOffset=0;screen=SD_TEXT;}else if(ext=="jpg"||ext=="jpeg"||ext=="png"||ext=="bmp"){path=fp;screen=IMAGE_VIEW;}else{message="Arquivo armazenado no SD.\nPDF/PPT/PPTX: transferencia disponivel.";}}break;}p=e+1;line++;}dirty=true;return;}
}
void move(int d){selected+=d;int max=0;if(screen==HOME)max=7;else if(screen==MATH)max=6;else if(screen==PHYSICS)max=6;else if(screen==CHEMISTRY)max=3;else if(screen==NOTES)max=2;else if(screen==SD_BROWSER||screen==BIOLOGY||screen==FILES){String l=SDManager::list(path);int n=0,p=0;while(p<l.length()){int e=l.indexOf('\n',p);if(e<0)e=l.length();n++;p=e+1;}max=max(0,n-1);}else max=0;if(selected<0)selected=max;if(selected>max)selected=0;dirty=true;}
void moveChem(int d){chemZ=(d<0)?(chemZ<=1?118:chemZ-1):(chemZ>=118?1:chemZ+1);dirty=true;}
void movePeriodic(int dx,int dy){const auto*e=Chemistry::byAtomicNumber(chemZ);int p=atoi(e->period),g=atoi(e->group);for(int t=0;t<30;t++){g+=dx;p+=dy;if(g<1)g=18;if(g>18)g=1;if(p<1)p=7;if(p>7)p=1;int z=zAt(p,g);if(z){chemZ=z;dirty=true;return;}}}
void key(){if(!M5Cardputer.Keyboard.isChange()||!M5Cardputer.Keyboard.isPressed())return;auto k=M5Cardputer.Keyboard.keysState();
 if(screen==SD_TEXT){for(auto ch:k.word){if(ch==';'||ch=='w'||ch=='W')textOffset=max(0,textOffset-120);else if(ch=='.'||ch=='s'||ch=='S')textOffset=min(max(0,(int)textCache.length()-1),textOffset+120);dirty=true;}if(k.del){back();return;}return;}
 if(screen==SD_BROWSER||screen==BIOLOGY||screen==FILES){for(auto ch:k.word){if(ch==';'||ch=='w'||ch=='W'){selected--;dirty=true;}else if(ch=='.'||ch=='s'||ch=='S'){selected++;dirty=true;}else if(ch==','){int slash=path.lastIndexOf('/');if(path!="/"){if(slash<=0)path="/";else path=path.substring(0,slash);selected=0;dirty=true;}}else if(ch=='/'){activate();return;}}if(k.del){if(path!="/"){int slash=path.lastIndexOf('/');if(slash<=0)path="/";else path=path.substring(0,slash);selected=0;dirty=true;}else back();return;}if(k.enter){activate();return;}return;}
 bool text=(screen==CALC||screen==EQUATION||screen==EXPONENTIAL||screen==TRIG||screen==PHYSICS_RUN||screen==NOTES_EDIT||(screen==GRAPH&&graphEditing));for(auto ch:k.word){if(text){appendInput(ch);continue;}if(screen==PERIODIC){if(ch==',')movePeriodic(-1,0);else if(ch=='/')movePeriodic(1,0);else if(ch==';')movePeriodic(0,-1);else if(ch=='.')movePeriodic(0,1);else if(ch=='w'||ch=='W')movePeriodic(0,-1);else if(ch=='s'||ch=='S')movePeriodic(0,1);}else if(screen==CHEM_INFO||screen==BONDS){if(ch==','||ch=='w'||ch=='W')moveChem(-1);else if(ch=='/'||ch=='s'||ch=='S')moveChem(1);}else if(screen==GRAPH){if(ch==',')graphCenter-=0.5;else if(ch=='/')graphCenter+=0.5;else if(ch==';')graphScale*=1.15;else if(ch=='.')graphScale/=1.15;dirty=true;}else if(ch==','||ch=='w'||ch=='W')move(-1);else if(ch=='/'||ch=='s'||ch=='S')move(1);}
 if(k.del){if(text&&input.length()){input.remove(input.length()-1);dirty=true;}else back();return;}if(k.enter){activate();return;}}
}
namespace App {void begin(){SDManager::begin();WifiManager::begin();dirty=true;draw();}void update(){key();WifiManager::update();draw();}}
