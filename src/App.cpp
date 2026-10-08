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
enum Screen { HOME,MATH,PHYSICS,CHEMISTRY,PERIODIC,BIOLOGY,FILES,WIFI,NOTES,CALC,LINEAR,QUADRATIC,EXPONENTIAL,TRIG,GRAPH,PHYS_WIZARD,CHEM_INFO,BONDS,SD_BROWSER,SD_TEXT,NOTES_EDIT };
Screen screen=HOME, previous=HOME; int selected=0; int offset=0; bool dirty=true; String input; String message; String path="/"; String textCache; int textOffset=0; uint8_t chemA=1,chemB=8; int physTarget=0; double graphX=-5,graphScale=1;
const char* home[] = {"∑","Δ","⚗","⚛","◉","▣","Wi","✎"};
const char* homeName[] = {"MATEMATICA","FISICA","QUIMICA","TABELA","BIOLOGIA","ARQUIVOS","CONEXAO","ANOTACOES"};
const char* mathName[] = {"CALCULADORA","1o GRAU","2o GRAU","EXPONENCIAL","TRIG/LOG","FUNCAO/GRAFICO"};
const char* physName[] = {"FORCA","VELOCIDADE","DISTANCIA","ACELERACAO","TEMPO","ENERGIA","POTENCIA","DENSIDADE","TRABALHO","ENERGIA POTENCIAL"};

void bg(){M5Cardputer.Display.fillScreen(TFT_BLACK);M5Cardputer.Display.setTextSize(1);}
void txt(const String&s,int x,int y,uint16_t c=TFT_WHITE){M5Cardputer.Display.setTextColor(c);M5Cardputer.Display.drawString(s,x,y);}
void header(const String&s){txt(s,5,3,TFT_CYAN);M5Cardputer.Display.drawFastHLine(4,16,232,TFT_DARKGREY);}
void card(int x,int y,int w,int h,const String&icon,const String&name,bool sel){if(sel){M5Cardputer.Display.fillRect(x,y,w,h,TFT_CYAN);txt(icon,x+4,y+5,TFT_BLACK);txt(name,x+4,y+h-12,TFT_BLACK);}else{M5Cardputer.Display.drawRect(x,y,w,h,TFT_DARKGREY);txt(icon,x+4,y+5,TFT_CYAN);txt(name,x+4,y+h-12,TFT_WHITE);}}
void footer(const String&s="← → navegar  ENTER abrir  DEL voltar"){txt(s,5,124,TFT_DARKGREY);}
void horizontal(const char**names,const char**icons,int n,const String&title){bg();header(title);int start=max(0,min(selected-1,n-3));offset=start;for(int i=0;i<3&&start+i<n;i++){int x=5+i*78;card(x,25,72,70,icons[start+i],names[start+i],start+i==selected);}footer();}
void drawHome(){bg();header("CENTRAL DE ESTUDOS");int start=max(0,min(selected-1,8-3));offset=start;for(int i=0;i<3&&start+i<8;i++){int idx=start+i;card(5+i*78,27,72,70,home[idx],homeName[idx],idx==selected);}txt("← → percorre as areas",5,108,TFT_GREEN);txt(SDManager::ready()?"SD OK":"SD ausente",170,108,SDManager::ready()?TFT_GREEN:TFT_RED);footer();}
void drawMath(){const char* ic[]={"∑","x","x²","aˣ","sin","ƒ"};horizontal(mathName,ic,6,"MATEMATICA");}
void drawPhys(){const char* ic[]={"F","v","d","a","t","E","P","ρ","W","Ep"};horizontal(physName,ic,10,"FISICA • SUPOSICAO");}
void drawChem(){const char* n[]={"TABELA","INFORMACOES","LIGACOES"};const char* i[]={"⚛","◎","⇄"};horizontal(n,i,3,"QUIMICA");}
void drawSimple(const String&t,const String&a,const String&b,const String&c){bg();header(t);txt("1",7,32,TFT_CYAN);txt(a,18,32);txt("2",7,52,TFT_CYAN);txt(b,18,52);txt("3",7,72,TFT_CYAN);txt(c,18,72);txt(message,7,94,TFT_YELLOW);footer();}
void drawInput(const String&t,const String&hint){bg();header(t);txt(hint,5,25,TFT_DARKGREY);M5Cardputer.Display.drawRect(5,43,230,22,TFT_DARKGREY);txt(input,9,49,TFT_WHITE);txt(message,5,76,TFT_YELLOW);txt("ENTER calcular  DEL voltar",5,108,TFT_DARKGREY);}
void drawCalc(){drawInput("CALCULADORA","Use pi, e, x, ^, sqrt(), sin(), cos(), tan(), ln(), log(), exp()");}
void drawSolver(const String&t,const String&hint){drawInput(t,hint);}
void drawGraph(){bg();header("GRAFICO • f(x) = "+input);int ox=120,oy=74;M5Cardputer.Display.drawFastHLine(5,oy,230,TFT_DARKGREY);M5Cardputer.Display.drawFastVLine(ox,22,100,TFT_DARKGREY);int px=-1,py=-1;for(int sx=5;sx<235;sx+=2){double x=(sx-ox)/18.0/graphScale+graphX;auto r=MathEngine::evaluate(input,x);if(!r.ok||fabs(r.value)>50){px=-1;continue;}int sy=oy-(int)(r.value*12*graphScale);if(sy<22||sy>122){px=-1;continue;}if(px>=0)M5Cardputer.Display.drawLine(px,py,sx,sy,TFT_GREEN);px=sx;py=sy;}txt("←/→ desloca   ↑/↓ zoom   DEL volta",5,124,TFT_DARKGREY);}
int zAt(int per,int grp){for(size_t i=0;i<Chemistry::count();++i){auto&e=Chemistry::all()[i];if(String(e.period).toInt()==per&&String(e.group).toInt()==grp)return e.z;}return 0;}
void periodicMove(int dx,int dy){auto* e=Chemistry::byAtomicNumber(chemA); if(!e)return; int per=String(e->period).toInt(), grp=String(e->group).toInt(); int np=per,ng=grp; for(int tries=0;tries<20;tries++){ng+=dx;np+=dy;if(ng<1)ng=18;if(ng>18)ng=1;if(np<1)np=7;if(np>7)np=1;int z=zAt(np,ng);if(z){chemA=z;dirty=true;return;}}}
void cell(int z,int x,int y,int cw,int ch,bool sel){if(!z)return;auto&e=*Chemistry::byAtomicNumber(z);if(sel)M5Cardputer.Display.fillRect(x,y,cw,ch,TFT_CYAN);else M5Cardputer.Display.drawRect(x,y,cw,ch,TFT_DARKGREY);txt(e.symbol,x+1,y+2,sel?TFT_BLACK:TFT_WHITE);}
void drawPeriodic(){bg();header("TABELA PERIODICA • ENTER = detalhes");int cw=12,ch=12,x0=5,y0=20;for(int p=1;p<=7;p++){for(int g=1;g<=18;g++){int z=zAt(p,g);bool sel=(z==(int)chemA);cell(z,x0+(g-1)*cw,y0+(p-1)*ch,cw,ch,sel);}}txt("LANTANIDEOS",5,107,TFT_DARKGREY);txt("ACTINIDEOS",5,118,TFT_DARKGREY);for(int i=0;i<15;i++){int z=58+i;cell(z,76+i*10,103,10,9,z==chemA);int za=90+i;cell(za,76+i*10,114,10,9,za==chemA);} }
void drawChemInfo(){auto e=Chemistry::byAtomicNumber(chemA);bg();header("ELEMENTO");if(!e){txt("Elemento inexistente",5,35);footer();return;}txt(String(e->symbol)+"  "+e->name,5,25,TFT_CYAN);txt("Z: "+String(e->z)+"  Grupo: "+e->group+"  Periodo: "+e->period,5,42);txt("Categoria: "+String(e->category),5,57);txt("Distribuicao: "+String(e->config),5,72);txt(Chemistry::octetNote(*e),5,87);txt("DEL volta",5,108,TFT_DARKGREY);}
void drawBonds(){auto a=Chemistry::byAtomicNumber(chemA),b=Chemistry::byAtomicNumber(chemB);bg();header("LIGACOES QUIMICAS");txt("A: "+String(a->symbol)+" "+a->name,5,27,TFT_CYAN);txt("B: "+String(b->symbol)+" "+b->name,5,44,TFT_CYAN);txt("Tipo: "+Chemistry::bondType(*a,*b),5,64,TFT_GREEN);txt("A distribuicao: "+String(a->config),5,80);txt("B distribuicao: "+String(b->config),5,94);txt("←/→ troca A  ↑/↓ troca B  ENTER alterna",5,112,TFT_DARKGREY);}
void drawSD(){bg();header("SD • "+path);String l=SDManager::list(path);int line=0;int pos=0;while(line<7&&pos<l.length()){int e=l.indexOf('\n',pos);if(e<0)e=l.length();txt(l.substring(pos,e),5,24+line*13);pos=e+1;line++;}footer("ENTER abre pasta/arquivo  DEL volta");}
void drawText(){bg();header(textCache.substring(0,28));int p=textOffset;for(int i=0;i<7&&p<textCache.length();i++){int e=textCache.indexOf('\n',p);if(e<0)e=textCache.length();String s=textCache.substring(p,e);if(s.length()>38)s=s.substring(0,38);txt(s,3,22+i*14);p=e+1;}footer("↑/↓ rolar  DEL voltar");}
void draw(){if(!dirty)return;dirty=false;switch(screen){case HOME:drawHome();break;case MATH:drawMath();break;case PHYSICS:drawPhys();break;case CHEMISTRY:drawChem();break;case PERIODIC:drawPeriodic();break;case CALC:drawCalc();break;case LINEAR:drawSolver("EQUACAO 1o GRAU","Digite a,b para ax+b=0");break;case QUADRATIC:drawSolver("EQUACAO 2o GRAU","Digite a,b,c para ax²+bx+c=0");break;case EXPONENTIAL:drawSolver("EXPONENCIAL","Digite a,b,c,d para a^(b*x+c)=d");break;case TRIG:drawSolver("TRIG / LOG / RAIZ","Ex.: sin(pi/2), log(100), sqrt(25)");break;case GRAPH:drawGraph();break;case PHYS_WIZARD:drawSolver("FISICA • SUPOSICAO",String("Formula: ")+Physics::formula(physTarget)+" | dados: m=2,a=3");break;case CHEM_INFO:drawChemInfo();break;case BONDS:drawBonds();break;case SD_BROWSER:drawSD();break;case SD_TEXT:drawText();break;case BIOLOGY:drawSimple("BIOLOGIA / SD","Celula e organelas","Fotossintese / respiracao","Mitoses / meiose / genetica");break;case FILES:drawSimple("ARQUIVOS","Navegar no SD","Abrir textos","Materiais enviados");break;case WIFI:drawSimple("CONEXAO WIFI","AP: Cardputer-Estudos","192.168.4.1","Upload para /ANOTACOES");break;case NOTES:drawSimple("ANOTACOES","Nova anotacao","Ler anotacoes","Arquivos no SD");break;case NOTES_EDIT:drawInput("NOVA ANOTACAO","Digite o texto; ENTER salva em /ANOTACOES/nota.txt");break;}}
void back(){if(screen==HOME)return; if(screen==SD_TEXT){screen=SD_BROWSER;dirty=true;return;}if(screen==SD_BROWSER){screen=HOME;selected=5;dirty=true;return;}if(screen==CALC||screen==LINEAR||screen==QUADRATIC||screen==EXPONENTIAL||screen==TRIG||screen==GRAPH){screen=MATH;selected=0;input="";message="";dirty=true;return;}if(screen==PHYS_WIZARD){screen=PHYSICS;selected=0;input="";dirty=true;return;}if(screen==CHEM_INFO||screen==BONDS){screen=CHEMISTRY;selected=0;dirty=true;return;}screen=HOME;selected=0;dirty=true;}
void inputAppend(char c){if(c>=32&&c<=126)input+=c;dirty=true;}
void activate(){
 if(screen==HOME){screen=(Screen)(MATH+selected); if(selected==3)screen=PERIODIC;if(selected==4)screen=BIOLOGY;if(selected==5)screen=SD_BROWSER;if(selected==6)screen=WIFI;if(selected==7)screen=NOTES;selected=0;input="";message="";dirty=true;return;}
 if(screen==MATH){screen=(Screen)(CALC+selected);selected=0;input="";message="";dirty=true;return;}
 if(screen==PHYSICS){physTarget=selected;screen=PHYS_WIZARD;input="";message="";dirty=true;return;}
 if(screen==CHEMISTRY){if(selected==0)screen=PERIODIC;else if(selected==1)screen=CHEM_INFO;else screen=BONDS;selected=0;dirty=true;return;}
 if(screen==PERIODIC){screen=CHEM_INFO;dirty=true;return;}
 if(screen==CALC){auto r=MathEngine::evaluate(input);message=r.text;dirty=true;return;}
 if(screen==LINEAR){int p=input.indexOf(',');if(p>0){message=MathEngine::solveLinear(input.substring(0,p).toDouble(),input.substring(p+1).toDouble()).text;}else message="Use a,b";dirty=true;return;}
 if(screen==QUADRATIC){int p=input.indexOf(','),q=input.indexOf(',',p+1);if(p>0&&q>p)message=MathEngine::solveQuadratic(input.substring(0,p).toDouble(),input.substring(p+1,q).toDouble(),input.substring(q+1).toDouble()).text;else message="Use a,b,c";dirty=true;return;}
 if(screen==EXPONENTIAL){int p=input.indexOf(','),q=input.indexOf(',',p+1),r=input.indexOf(',',q+1);if(p>0&&q>p&&r>q)message=MathEngine::solveExponential(input.substring(0,p).toDouble(),input.substring(p+1,q).toDouble(),input.substring(q+1,r).toDouble(),input.substring(r+1).toDouble()).text;else message="Use a,b,c,d";dirty=true;return;}
 if(screen==TRIG){auto r=MathEngine::evaluate(input);message=r.text;dirty=true;return;}
 if(screen==GRAPH){return;}
 if(screen==PHYS_WIZARD){message=Physics::solve(physTarget,input);dirty=true;return;}
 if(screen==BONDS){chemB=chemB==118?1:chemB+1;dirty=true;return;}
 if(screen==SD_BROWSER){String l=SDManager::list(path);int p=0;for(int i=0;i<=selected&&p<l.length();i++){int e=l.indexOf('\n',p);if(e<0)e=l.length();if(i==selected){String n=l.substring(p,e);if(n.startsWith("[D] ")){path=SDManager::normalize(path,n.substring(4));selected=0;}else{String fn=n.substring(4);textCache=SDManager::readText(SDManager::normalize(path,fn));textOffset=0;screen=SD_TEXT;}break;}p=e+1;}dirty=true;return;}
 if(screen==NOTES){screen=NOTES_EDIT;input="";message="";dirty=true;return;}
 if(screen==NOTES_EDIT){if(input.length()){File f=SD.open("/ANOTACOES/nota.txt","w");if(f){f.print(input);f.close();message="Salvo em /ANOTACOES/nota.txt";}else message="Falha ao salvar.";}dirty=true;return;}
}
void key(){if(!M5Cardputer.Keyboard.isChange()||!M5Cardputer.Keyboard.isPressed())return;auto k=M5Cardputer.Keyboard.keysState();bool text=(screen==CALC||screen==LINEAR||screen==QUADRATIC||screen==EXPONENTIAL||screen==TRIG||screen==PHYS_WIZARD||screen==NOTES_EDIT);for(auto ch:k.word){if(text){inputAppend(ch);continue;}if(ch==','){if(screen==PERIODIC)periodicMove(-1,0);else if(screen==BONDS){chemA=chemA<=1?118:chemA-1;dirty=true;}else{selected--;dirty=true;}}else if(ch=='/'){if(screen==PERIODIC)periodicMove(1,0);else if(screen==BONDS){chemA=chemA>=118?1:chemA+1;dirty=true;}else{selected++;dirty=true;}}else if(ch==';'){if(screen==PERIODIC)periodicMove(0,-1);else{selected--;dirty=true;}}else if(ch=='.'){if(screen==PERIODIC)periodicMove(0,1);else{selected++;dirty=true;}}else if(ch=='w'||ch=='W'){selected--;dirty=true;}else if(ch=='s'||ch=='S'){selected++;dirty=true;}}
 if(!text && screen!=PERIODIC && screen!=BONDS && screen!=GRAPH){int max=0;if(screen==HOME)max=7;else if(screen==MATH)max=5;else if(screen==PHYSICS)max=9;else if(screen==CHEMISTRY)max=2;else max=2;if(selected<0)selected=max;if(selected>max)selected=0;dirty=true;}
 if(k.del){back();return;} if(k.enter){activate();return;}
 if(screen==GRAPH){for(auto ch:k.word){if(ch==',')graphX-=0.5;else if(ch=='/')graphX+=0.5;else if(ch==';')graphScale*=1.15;else if(ch=='.')graphScale/=1.15;}dirty=true;}
}
}
namespace App {void begin(){SDManager::begin();WifiManager::begin();dirty=true;draw();}void update(){key();WifiManager::update();draw();}}
