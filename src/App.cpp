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
enum Screen { HOME,MATH,PHYSICS,CHEMISTRY,PERIODIC,CHEM_INFO,BONDS,BIOLOGY,FILES,WIFI,NOTES,SD_BROWSER,SD_TEXT,IMAGE_VIEW,CALC,EQUATION,EXPONENTIAL,TRIG,GRAPH,PHYSICS_RUN,PHYSICS_ASSIST,PHYSICS_DATA,PHYSICS_VALUES,PHYSICS_RESULT,NOTES_EDIT,RENAME };
Screen screen=HOME;
int selected=0;
bool dirty=true;
String input="";
String message="";
String path="/";
String textCache="";
int textOffset=0;
String renameOld="";
String renameExt="";
uint8_t chemZ=1,chemB=8;
uint8_t physTarget=0;
uint8_t assistCategory=0;
uint8_t assistVarCount=0;
uint8_t assistVarPos=0;
bool assistSelected[8]={false,false,false,false,false,false,false,false};
String assistValues[8];
String assistVars[8];
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
void drawWrappedMessage(const String&src,int x,int y,int maxLines,uint16_t c);
void drawPhysRun(){bg();header(String("FISICA • ")+Physics::name(physTarget));txt("Formula sugerida:",5,24,TFT_DARKGREY);txt(Physics::formula(physTarget),5,38,TFT_CYAN);txt("Dados: m=2,a=3",5,52,TFT_DARKGREY);M5Cardputer.Display.drawRect(5,61,230,23,TFT_DARKGREY);String shown=input;if(shown.length()>35)shown=shown.substring(shown.length()-35);txt(shown,9,68);drawWrappedMessage(message,5,90,2,TFT_YELLOW);footer("ENTER calcular   DEL voltar");}
void drawWrappedMessage(const String&src,int x,int y,int maxLines,uint16_t c){int p=0;for(int line=0;line<maxLines&&p<src.length();line++){while(p<src.length()&&(src[p]=='\n'||src[p]=='\r'))p++;if(p>=src.length())break;int end=p,chars=0,lastSpace=-1;while(end<src.length()&&src[end]!='\n'&&src[end]!='\r'&&chars<38){if(src[end]==' ')lastSpace=end;end++;chars++;}if(end<src.length()&&src[end]!='\n'&&lastSpace>p)end=lastSpace;String part=src.substring(p,end);part.trim();txt(part,x,y+line*13,c);p=end;if(p<src.length()&&src[p]==' ')p++;} }
void advanceToSelectedAssistVar(){while(assistVarPos<assistVarCount&&!assistSelected[assistVarPos])assistVarPos++;}
void setAssistVars(){assistVarCount=0;String v;switch(assistCategory){case 0:v="F1";assistVars[assistVarCount++]=v;assistVars[assistVarCount++]="F2";assistVars[assistVarCount++]="F3";assistVars[assistVarCount++]="m";assistVars[assistVarCount++]="a";break;case 1:assistVars[assistVarCount++]="m";assistVars[assistVarCount++]="g";break;case 2:assistVars[assistVarCount++]="m";assistVars[assistVarCount++]="g";assistVars[assistVarCount++]="theta";break;case 3:assistVars[assistVarCount++]="mu";assistVars[assistVarCount++]="N";assistVars[assistVarCount++]="m";assistVars[assistVarCount++]="g";break;case 4:assistVars[assistVarCount++]="k";assistVars[assistVarCount++]="x";break;case 5:assistVars[assistVarCount++]="vi";assistVars[assistVarCount++]="vf";assistVars[assistVarCount++]="a";assistVars[assistVarCount++]="t";assistVars[assistVarCount++]="d";assistVars[assistVarCount++]="v";break;case 6:assistVars[assistVarCount++]="F";assistVars[assistVarCount++]="d";assistVars[assistVarCount++]="theta";assistVars[assistVarCount++]="m";assistVars[assistVarCount++]="v";assistVars[assistVarCount++]="g";assistVars[assistVarCount++]="h";break;case 7:assistVars[assistVarCount++]="m";assistVars[assistVarCount++]="V";break;}for(int i=0;i<8;i++){assistSelected[i]=false;assistValues[i]="";}assistVarPos=0;advanceToSelectedAssistVar();input="";message="";}
void drawPhysicsAssist(){bg();header("ASSISTENTE DE FISICA");txt("O que voce esta procurando?",5,23,TFT_WHITE);const char* n[]={"Forca resultante","Forca peso","Forca normal","Atrito","Forca elastica","Cinematica","Energia / trabalho","Densidade"};int start=selected;if(start>5)start=5;for(int i=0;i<3&&start+i<8;i++){int idx=start+i;M5Cardputer.Display.fillRoundRect(5,39+i*25,230,21,3,idx==selected?TFT_CYAN:TFT_DARKGREY);txt(String(idx+1)+"  "+n[idx],10,45+i*25,idx==selected?TFT_BLACK:TFT_WHITE);}footer("↑↓ escolher   ENTER continuar   DEL voltar");}
void drawPhysicsData(){bg();header(String("DADOS • ")+Physics::name(assistCategory));txt("Marque somente o que voce possui:",5,23,TFT_DARKGREY);int start=selected;if(start>assistVarCount-3)start=max(0,(int)assistVarCount-3);for(int i=0;i<3&&start+i<assistVarCount;i++){int idx=start+i;String mark=assistSelected[idx]?"[X] ":"[ ] ";txt(mark+assistVars[idx],7,43+i*22,idx==selected?TFT_CYAN:TFT_WHITE);}txt("C = confirmar dados",5,111,TFT_YELLOW);footer("↑↓ navegar   ENTER marcar   DEL voltar");}
void drawPhysicsValues(){bg();header(String("VALORES • ")+Physics::name(assistCategory));String label=assistVars[assistVarPos];txt("Informe o valor de "+label+":",5,27,TFT_WHITE);M5Cardputer.Display.drawRoundRect(5,39,230,27,4,TFT_DARKGREY);String shown=input;if(shown.length()>34)shown=shown.substring(shown.length()-34);txt(shown,9,48,TFT_CYAN);txt("ENTER salvar e proximo",5,80,TFT_DARKGREY);txt(String(assistVarPos+1)+" / "+String(assistVarCount),195,80,TFT_DARKGREY);footer("Digite o numero   DEL voltar");}
void drawPhysicsResult(){bg();header("RESULTADO • ASSISTENTE");drawWrappedMessage(message,5,24,7,TFT_WHITE);footer("DEL voltar   ENTER novo calculo");}
void drawSD(){bg();header("SD • "+path);String l=SDManager::list(path);int n=0,p=0;while(p<l.length()){int e=l.indexOf('\n',p);if(e<0)e=l.length();n++;p=e+1;}int start=selected-6;if(start<0)start=0;int line=0;p=0;while(p<l.length()&&line<start+7){int e=l.indexOf('\n',p);if(e<0)e=l.length();if(line>=start){String s=l.substring(p,e);int row=line-start;if(line==selected)M5Cardputer.Display.fillRect(2,22+row*14,236,13,TFT_CYAN);txt(s,5,24+row*14,line==selected?TFT_BLACK:TFT_WHITE);}p=e+1;line++;}footer("ENTER abrir   ↑↓ selecionar   N renomear");}
void getWrappedLine(const String&src,int wanted,String&out){int p=0,line=0;while(p<src.length()){while(p<src.length()&&(src[p]=='\n'||src[p]=='\r')){if(src[p]=='\n')line++;p++;}if(p>=src.length())break;int end=p,chars=0,lastSpace=-1;while(end<src.length()&&src[end]!='\n'&&src[end]!='\r'&&chars<38){if(src[end]==' ')lastSpace=end;end++;chars++;}if(end<src.length()&&src[end]!='\n'&&lastSpace>p)end=lastSpace;String part=src.substring(p,end);part.trim();if(line==wanted){out=part;return;}line++;p=end;if(p<src.length()&&src[p]==' ')p++;}out="";}
void drawText(){bg();header("TEXTO • SD");for(int i=0;i<7;i++){String line;getWrappedLine(textCache,textOffset+i,line);if(line.length())txt(line,3,21+i*14,TFT_WHITE);}txt("linha "+String(textOffset+1),190,7,TFT_DARKGREY);footer("↑↓ rolar   DEL voltar");}
String extension(const String&p){int i=p.lastIndexOf('.');if(i<0)return "";String e=p.substring(i+1);e.toLowerCase();return e;}

void drawImage(){
 bg();
 header("IMAGEM • SD");
 String ext=extension(path);

 // M5GFX 0.2.32 can have a DataWrapper/SDFS compatibility issue
 // with draw*File(SD, ...). Read the image into RAM and use the
 // memory-based decoder instead.
 if(ext=="jpg"||ext=="jpeg"||ext=="png"||ext=="bmp"){
   File f=SD.open(path,"r");
   if(!f){
     txt("Nao foi possivel abrir a imagem",5,40,TFT_RED);
     footer("DEL voltar");
     return;
   }

   size_t len=f.size();
   if(len==0 || len>600000){
     f.close();
     txt("Imagem muito grande ou vazia",5,40,TFT_RED);
     footer("DEL voltar");
     return;
   }

   uint8_t* data=(uint8_t*)malloc(len);
   if(!data){
     f.close();
     txt("Memoria insuficiente para a imagem",5,40,TFT_RED);
     footer("DEL voltar");
     return;
   }

   size_t readLen=f.read(data,len);
   f.close();

   bool ok=false;
   if(readLen==len){
     if(ext=="jpg"||ext=="jpeg")
       ok=M5Cardputer.Display.drawJpg(data,len,0,18,240,103);
     else if(ext=="png")
       ok=M5Cardputer.Display.drawPng(data,len,0,18,240,103);
     else if(ext=="bmp")
       ok=M5Cardputer.Display.drawBmp(data,len,0,18,240,103);
   }

   free(data);

   if(!ok) txt("Nao foi possivel decodificar a imagem",5,40,TFT_RED);
 }else{
   txt("Formato de imagem nao suportado",5,40,TFT_RED);
 }
 footer("DEL voltar");
}

void drawGraph(){
 bg();
 header("GRAFICO / FUNCAO");

 String expr=input;
 if(expr.length()==0)expr="x^2";

 const int gx=4,gy=22,gw=232,gh=96;
 float xRange=max(0.5,graphScale);
 float yRange=max(0.5,graphScale);
 float x0=graphCenter-xRange;
 float x1=graphCenter+xRange;
 float y0=-yRange;
 float y1=yRange;

 M5Cardputer.Display.drawRect(gx,gy,gw,gh,TFT_DARKGREY);

 int axisX=(int)round(gx+((0.0-x0)/(x1-x0))*(gw-1));
 int axisY=(int)round(gy+((y1-0.0)/(y1-y0))*(gh-1));

 if(axisX>=gx&&axisX<gx+gw)
   M5Cardputer.Display.drawFastVLine(axisX,gy,gh,TFT_DARKGREY);

 if(axisY>=gy&&axisY<gy+gh)
   M5Cardputer.Display.drawFastHLine(gx,axisY,gw,TFT_DARKGREY);

 bool havePrev=false;
 int prevX=0,prevY=0;

 for(int px=0;px<gw;px++){
   double x=x0+(x1-x0)*(double)px/(double)(gw-1);
   auto r=MathEngine::evaluate(expr,x);

   if(!r.ok||!isfinite(r.value)){
     havePrev=false;
     continue;
   }

   double y=r.value;

   if(y<y0||y>y1){
     havePrev=false;
     continue;
   }

   int py=(int)round(gy+((y1-y)/(y1-y0))*(gh-1));

   if(havePrev&&abs(py-prevY)<gh)
     M5Cardputer.Display.drawLine(prevX,prevY,gx+px,py,TFT_CYAN);
   else
     M5Cardputer.Display.drawPixel(gx+px,py,TFT_CYAN);

   prevX=gx+px;
   prevY=py;
   havePrev=true;
 }

 txt("f(x)="+expr,5,120,TFT_WHITE);
 footer("ENTER editar   DEL voltar");
}

void drawRename(){bg();header("RENOMEAR ARQUIVO • N");txt("Atual: "+renameOld,5,25,TFT_DARKGREY);txt("Novo nome (sem trocar extensao):",5,41,TFT_WHITE);M5Cardputer.Display.drawRoundRect(4,52,232,27,4,TFT_DARKGREY);String shown=input;if(shown.length()>34)shown=shown.substring(shown.length()-34);txt(shown+renameExt,8,61,TFT_CYAN);txt("ENTER confirmar   DEL cancelar",5,100,TFT_DARKGREY);}
void drawNotesMenu(){bg();header("ANOTACOES");card(5,27,72,72,"+","NOVA",selected==0,TFT_GREEN);card(83,27,72,72,"SD","LER",selected==1,TFT_CYAN);card(161,27,72,72,"N","MATERIAIS",selected==2,TFT_YELLOW);footer();}
void draw(){if(!dirty)return;dirty=false;switch(screen){case HOME:drawHome();break;case MATH:drawMath();break;case PHYSICS:drawPhys();break;case CHEMISTRY:drawChem();break;case PERIODIC:drawPeriodic();break;case CHEM_INFO:drawChemInfo();break;case BONDS:drawBonds();break;case BIOLOGY:path="/BIOLOGIA";drawSD();break;case FILES:drawSD();break;case WIFI:bg();header("WI-FI • TRANSFERENCIA");txt("Rede: Cardputer-Estudos",5,30,TFT_CYAN);txt("Endereco: 192.168.4.1",5,47);txt("Envie arquivos pelo navegador",5,64);txt("Arquivos ficam no SD",5,81,TFT_GREEN);footer("DEL voltar");break;case NOTES:drawNotesMenu();break;case SD_BROWSER:drawSD();break;case SD_TEXT:drawText();break;case IMAGE_VIEW:drawImage();break;case CALC:drawCalc();break;case EQUATION:drawEquation("RESOLVER EQUACAO");break;case EXPONENTIAL:drawEquation("EQUACAO EXPONENCIAL");break;case TRIG:drawInput("TRIG / LOG / RAIZ","Ex.: sen(pi/2)   log(100)   sqrt(25)");break;case GRAPH:drawGraph();break;case PHYSICS_RUN:drawPhysRun();break;case PHYSICS_ASSIST:drawPhysicsAssist();break;case PHYSICS_DATA:drawPhysicsData();break;case PHYSICS_VALUES:drawPhysicsValues();break;case PHYSICS_RESULT:drawPhysicsResult();break;case NOTES_EDIT:drawInput("NOVA ANOTACAO","Digite e ENTER salva no SD.");break;case RENAME:drawRename();break;}}
void goHome(){screen=HOME;selected=0;input="";message="";path="/";dirty=true;}
void back(){if(screen==HOME)return;if(screen==SD_TEXT||screen==IMAGE_VIEW){screen=SD_BROWSER;dirty=true;return;}if(screen==SD_BROWSER){goHome();return;}if(screen==PERIODIC||screen==CHEM_INFO||screen==BONDS){screen=CHEMISTRY;selected=0;dirty=true;return;}if(screen==CALC||screen==EQUATION||screen==EXPONENTIAL||screen==TRIG||screen==GRAPH){screen=MATH;selected=0;input="";message="";dirty=true;return;}if(screen==PHYSICS_RUN||screen==PHYSICS_ASSIST||screen==PHYSICS_DATA||screen==PHYSICS_VALUES||screen==PHYSICS_RESULT){screen=PHYSICS;selected=0;input="";message="";dirty=true;return;}if(screen==NOTES_EDIT){screen=NOTES;dirty=true;return;}if(screen==RENAME){screen=SD_BROWSER;input="";renameOld="";renameExt="";dirty=true;return;}goHome();}
void appendInput(char c){if(c=='`')return;if(c==',')c='.';if(c>=32&&c<=126){input+=c;dirty=true;}}

void activate(){
 if(screen==HOME){switch(selected){case 0:screen=MATH;break;case 1:screen=PHYSICS;break;case 2:screen=CHEMISTRY;break;case 3:screen=PERIODIC;chemZ=1;break;case 4:screen=BIOLOGY;path="/BIOLOGIA";break;case 5:screen=FILES;path="/";break;case 6:screen=WIFI;break;case 7:screen=NOTES;break;}selected=0;input="";message="";dirty=true;return;}
 if(screen==MATH){switch(selected){case 0:screen=CALC;break;case 1:screen=EQUATION;break;case 2:screen=EXPONENTIAL;break;case 3:screen=TRIG;break;case 4:screen=GRAPH;input="x^2";graphEditing=false;break;case 5:screen=SD_BROWSER;path="/MATEMATICA";break;case 6:screen=SD_BROWSER;path="/MATEMATICA";break;}selected=0;input=screen==GRAPH?input:"";message="";dirty=true;return;}
 if(screen==PHYSICS){if(selected==6){screen=SD_BROWSER;path="/FISICA";}else if(selected==0){assistCategory=0;selected=0;setAssistVars();screen=PHYSICS_ASSIST;}else{physTarget=selected;screen=PHYSICS_RUN;input="";message="";}dirty=true;return;}
 if(screen==CHEMISTRY){if(selected==0)screen=PERIODIC;else if(selected==1)screen=CHEM_INFO;else if(selected==2)screen=BONDS;else {screen=SD_BROWSER;path="/QUIMICA";}selected=0;dirty=true;return;}
 if(screen==PERIODIC){screen=CHEM_INFO;dirty=true;return;}
 if(screen==CHEM_INFO){screen=BONDS;dirty=true;return;}
 if(screen==CALC||screen==TRIG){auto r=MathEngine::evaluate(input);message=r.text;dirty=true;return;}
 if(screen==EQUATION){auto r=MathEngine::solveEquation(input);message=r.text;dirty=true;return;}
 if(screen==EXPONENTIAL){auto r=MathEngine::solveExponential(input);message=r.text;dirty=true;return;}
 if(screen==GRAPH){graphEditing=!graphEditing;dirty=true;return;}
 if(screen==PHYSICS_ASSIST){assistCategory=selected;setAssistVars();selected=0;screen=PHYSICS_DATA;dirty=true;return;}
 if(screen==PHYSICS_DATA){if(assistVarCount==0){message="Nenhum dado disponivel.";screen=PHYSICS_RESULT;dirty=true;return;}if(assistSelected[selected])assistSelected[selected]=false;else assistSelected[selected]=true;dirty=true;return;}
 if(screen==PHYSICS_VALUES){advanceToSelectedAssistVar();if(assistVarPos>=assistVarCount){message="Nenhum dado foi preenchido.";screen=PHYSICS_RESULT;dirty=true;return;}if(input.length()==0){message="Digite um valor para "+assistVars[assistVarPos];dirty=true;return;}assistValues[assistVarPos]=input;input="";assistVarPos++;advanceToSelectedAssistVar()
