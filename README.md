I am not offering any license for this project, so according copyright lows without my permission nobody else can alter, modify, distribute or reuse any part of this project without being at risk of litigation.
Contact info: Vahagn Nurijanyan, vnurin@gmail.com.

This project is based on new data/text compression technique/algorithms which always provide better compression rate than the techniques based on LZ77 (zip & png file formats).
#
"datar <filename>" -> <filename>.dtz & this is for data having 1 byte symbols
"datar <filename> "z" "8"/"16"/"32" -> <filename>.dtz & this is for data having 1/2/4 byte symbols respectively
"datar <filename>.dtz" -> <filename>

Variations
1. !defined COMPLEX:
//#define COMPLEX
#ifdef COMPLEX
#define TABLES
#ifdef TABLES
#define HUFFMAN
#endif
#endif

2. defined COMPLEX:
#define COMPLEX
#ifdef COMPLEX
//#define TABLES
#ifdef TABLES
#define HUFFMAN
#endif
#endif

3. defined COMPLEX & defined TABLES:
#define COMPLEX
#ifdef COMPLEX
#define TABLES
#ifdef TABLES
//#define HUFFMAN
#endif
#endif

4. defined COMPLEX & defined TABLES & defined HUFFMAN:
#define COMPLEX
#ifdef COMPLEX
#define TABLES
#ifdef TABLES
#define HUFFMAN
#endif
#endif

Here are compression rates for the 4 variations:

CPU: 2.3 GHz Quad-Core Intel Core i7. CPU Time includes also reading / writing time of data of storage!

Canterbury Corpus

Name		Size		Compressed Size			bps			Compression CPU Time			Decompression CPU Time

alice29.txt     152089		56963/56579/56720/56176		3.00/2.98/2.98/2.95	0.626719/0.738249/0.720062/0.614392	0.065629/0.183989/0.178018/0.082975
asyoulik.txt	125179		51944/51224/51459/50605		3.32/3.27/3.29/3.23	0.478429/0.569352/0.58178/0.542028	0.054612/0.16166/0.153027/0.06943
cp.html		24603		10042/10566/10749/10857		3.26/3.43/3.50/3.53	0.087984/0.13094/0.110119/0.100165	0.011936/0.057545/0.037953/0.018637
fields.c	11150		3994/4168/4293/4446		2.86/3.00/3.08/3.19	0.045369/0.064719/0.058697/0.052101	0.007015/0.024985/0.014236/0.009465
grammar.lsp	3721		1573/1625/1801/1933		3.38/3.50/3.87/4.15	0.045369/0.028215/0.018342/0.016759	0.002838/0.008733/0.005495/0.004735
kennedy.xls	1029744		296410/294665/291995/290247	2.30/2.29/2.27/2.25	0.016281/7.07881/7.26053/4.03069	0.089154/10.3422/11.0797/0.274442
lcet10.txt	426754		145609/142959/143079/141920	2.73/2.68/2.68/2.66	3.91753/2.00833/2.06224/1.86961		0.211339/0.425088/0.427041/0.209289
plrabn12.txt	481861		192650/189280/189839/185796	3.20/3.14/3.15/3.08	1.79171/2.32453/2.36071/2.14863		0.267994/0.489374/0.488094/0.248863
ptt5		513216		59207/60536/60944/60475		0.92/0.94/0.95/0.94	2.08083/2.07091/2.00856/1.91647		0.192483/0.395434/0.362808/0.004735
sum		38240		17385/18892/19290/19697		3.64/3.95/4.03/4.12	0.148881/0.314921/0.217353/0.165206	0.020875/0.218419/0.10997/0.035989
xargs.1		4227		2152/2259/2438/2572		4.07/4.27/4.61/4.87	0.017798/0.028255/0.019223/0.018568	0.002942/0.013199/0.007036/0.00511
all_appended	2810784		838334/831013/832712/826762	2.39/2.36/2.37/2.35	11.3031/15.5403/15.6577/11.6207		1.01141/13.082/13.2577/1.06853

Large Canterbury Corpus

Name		Size		Compressed Size			bps			Compression CPU Time			Decompression CPU Time

E.coli		4638690		1349831/1326906/1326909/1349805	2.33/2.29/2.29/2.33	22.9314/20.4903/20.2702/20.4231		9.75071/3.76727/3.72764/3.62907
bible.txt	4047392		1095985/1079326/1080751/1062381	2.17/2.13/2.14/2.10	17.9686/18.241/18.213/17.3964		3.98604/3.13363/3.15918/1.87721
world192.txt	2473400		609863/598821/600455/591471	1.97/1.94/1.94/1.91	10.8051/11.354/11.4946/10.7601		1.80192/2.28649/2.36344/1.25202

Protein Corpus

Name		Size		Compressed Size			bps			Compression CPU Time			Decompression CPU Time

hi		509519		315472/311624/311700/316626	4.95/4.89/4.89/5.0	2.33204/2.48399/2.58264/2.44081		0.288958/0.481931/0.48065/0.254064
hs		3295751		1965038/1943237/1943319/196379	4.77/4.72/4.72/4.77	17.5442/16.3826/16.2935/15.7521		6.8275/3.15164/3.17175/1.92022
mj		448779		270820/267978/268053/273570	4.83/4.78/4.78/4.88	1.99742/2.17188/2.22432/2.09225		0.240615/0.413132/0.425554/0.226191
sc		2900352		1739055/1716856/1716937/1734619	4.80/4.73/4.73/4.78	15.0552/14.5554/14.4549/13.9552		5.41638/2.88684/2.83008/1.68311
