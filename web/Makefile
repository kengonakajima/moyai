all:
	browserify modsetup.js -o modbundle.js --s Snappy

wsf:
	node node_modules/websockify/websockify.js 8888 localhost:22222

toarray:
	node toarray.js ../assets/base.png > assets/base.png.js
	node toarray.js ../assets/blobloblll.wav  > assets/blobloblll.wav.js
	node toarray.js ../assets/cinecaption227.ttf > assets/cinecaption227.ttf.js
	node toarray.js ../assets/dragon8.png > assets/dragon8.png.js
	node toarray.js ../assets/font_only.png > assets/font_only.png.js
	node toarray.js ../assets/gymno1short.wav > assets/gymno1short.wav.js

