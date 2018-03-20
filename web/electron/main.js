'use strict';

var electron = require("electron");
var app = electron.app;
var BrowserWindow = electron.BrowserWindow;

var mainWindow = null;

app.on("window-all-closed", function() {
    console.log("window-all-closed");
    app.quit();
});


app.on("ready", function() {
    console.log("dirname:",__dirname);
    mainWindow = new BrowserWindow( {width:800, height:600} ) ;
    mainWindow.loadURL("file://" + __dirname + "/min2d.html");
    mainWindow.on("closed", function() {
        mainWindow=null;
    });
});