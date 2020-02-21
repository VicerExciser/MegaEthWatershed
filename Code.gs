/*--------------------------------------------------------------------
----------------------------------------------------------------------
Author: Austin Condict

This Google Apps Script was written for Atlanta's Halfway Crooks brewery
to log wastewater discharge flume levels measured once every 15 seconds.

The script automates generating & formatting a new Google Sheet for each
new month & year, with the name of the Sheet corresponding accordingly:
    "{current month}/{current year}"
----------------------------------------------------------------------
Current data variables being transmitted for the Spreadsheet (in order):
    voltageData
    levelData
    phData
----------------------------------------------------------------------
API docs:
    https://developers.google.com/apps-script/reference/spreadsheet/spreadsheet-app
    https://developers.google.com/apps-script/reference/spreadsheet/spreadsheet
    https://developers.google.com/apps-script/reference/spreadsheet/sheet
    https://developers.google.com/apps-script/reference/spreadsheet/range
----------------------------------------------------------------------
Test URL for posting dummy data to Sheet via PushingBox API (enter into browser):
    http://api.pushingbox.com/pushingbox?devid=v18CD7A17B3D3A00&json={%222%2f20%2f2020%2c10%3a39%3a14+PM%22%3a%221.04%2c0.00%2c6.17%22%2c%222%2f20%2f2020%2c10%3a39%3a29+PM%22%3a%221.09%2c0.00%2c6.13%22%2c%222%2f20%2f2020%2c10%3a39%3a44+PM%22%3a%221.12%2c0.00%2c6.15%22%2c%222%2f20%2f2020%2c10%3a39%3a59+PM%22%3a%221.14%2c0.00%2c6.18%22%2c%222%2f20%2f2020%2c10%3a40%3a15+PM%22%3a%221.15%2c0.00%2c6.10%22%2c%222%2f20%2f2020%2c10%3a40%3a30+PM%22%3a%221.16%2c0.00%2c6.13%22%2c%222%2f20%2f2020%2c10%3a40%3a45+PM%22%3a%221.17%2c0.00%2c6.10%22%2c%222%2f20%2f2020%2c10%3a41%3a00+PM%22%3a%221.18%2c0.00%2c6.13%22%2c%222%2f20%2f2020%2c10%3a41%3a15+PM%22%3a%221.20%2c0.00%2c6.18%22%2c%222%2f20%2f2020%2c10%3a41%3a31+PM%22%3a%221.21%2c0.00%2c6.10%22%2c%222%2f20%2f2020%2c10%3a41%3a46+PM%22%3a%221.21%2c0.00%2c6.13%22%2c%222%2f20%2f2020%2c10%3a42%3a01+PM%22%3a%221.22%2c0.00%2c6.18%22%2c%222%2f20%2f2020%2c10%3a42%3a16+PM%22%3a%221.22%2c0.00%2c6.18%22%2c%222%2f20%2f2020%2c10%3a42%3a31+PM%22%3a%221.22%2c0.00%2c6.10%22%2c%222%2f20%2f2020%2c10%3a42%3a46+PM%22%3a%221.22%2c0.00%2c6.10%22%2c%222%2f20%2f2020%2c10%3a43%3a01+PM%22%3a%221.22%2c0.00%2c6.10%22%2c%222%2f20%2f2020%2c10%3a43%3a17+PM%22%3a%221.22%2c0.00%2c6.16%22%2c%222%2f20%2f2020%2c10%3a43%3a32+PM%22%3a%221.22%2c0.00%2c6.10%22%2c%222%2f20%2f2020%2c10%3a43%3a47+PM%22%3a%221.22%2c0.00%2c6.16%22%2c%222%2f20%2f2020%2c10%3a44%3a02+PM%22%3a%221.22%2c0.00%2c6.13%22%2c%222%2f20%2f2020%2c10%3a44%3a17+PM%22%3a%221.22%2c0.00%2c6.10%22%2c%222%2f20%2f2020%2c10%3a44%3a32+PM%22%3a%221.22%2c0.00%2c6.10%22%2c%222%2f20%2f2020%2c10%3a44%3a47+PM%22%3a%221.22%2c0.00%2c6.18%22%2c%222%2f20%2f2020%2c10%3a45%3a03+PM%22%3a%221.21%2c0.00%2c6.18%22%2c%222%2f20%2f2020%2c10%3a45%3a18+PM%22%3a%221.22%2c0.00%2c6.10%22%2c%222%2f20%2f2020%2c10%3a45%3a33+PM%22%3a%221.22%2c0.00%2c6.18%22%2c%222%2f20%2f2020%2c10%3a45%3a48+PM%22%3a%221.22%2c0.00%2c6.18%22%2c%222%2f20%2f2020%2c10%3a46%3a03+PM%22%3a%221.22%2c0.00%2c6.13%22%2c%222%2f20%2f2020%2c10%3a46%3a19+PM%22%3a%221.22%2c0.00%2c6.18%22%2c%222%2f20%2f2020%2c10%3a46%3a34+PM%22%3a%221.22%2c0.00%2c6.13%22%2c%222%2f20%2f2020%2c10%3a46%3a49+PM%22%3a%221.22%2c0.00%2c6.13%22%2c%222%2f20%2f2020%2c10%3a47%3a04+PM%22%3a%221.23%2c0.00%2c6.13%22%2c%222%2f20%2f2020%2c10%3a47%3a19+PM%22%3a%221.23%2c0.00%2c6.10%22%2c%222%2f20%2f2020%2c10%3a47%3a34+PM%22%3a%221.23%2c0.00%2c6.18%22%2c%222%2f20%2f2020%2c10%3a47%3a49+PM%22%3a%221.23%2c0.00%2c6.18%22%2c%222%2f20%2f2020%2c10%3a48%3a05+PM%22%3a%221.23%2c0.00%2c6.18%22%2c%222%2f20%2f2020%2c10%3a48%3a20+PM%22%3a%221.23%2c0.00%2c6.13%22%2c%222%2f20%2f2020%2c10%3a48%3a35+PM%22%3a%221.22%2c0.00%2c6.18%22%2c%222%2f20%2f2020%2c10%3a48%3a50+PM%22%3a%221.22%2c0.00%2c6.16%22%2c%222%2f20%2f2020%2c10%3a49%3a05+PM%22%3a%221.22%2c0.00%2c6.16%22%2c%222%2f20%2f2020%2c10%3a49%3a20+PM%22%3a%221.21%2c0.00%2c6.16%22%2c%222%2f20%2f2020%2c10%3a49%3a35+PM%22%3a%221.21%2c0.00%2c6.18%22%2c%222%2f20%2f2020%2c10%3a49%3a51+PM%22%3a%221.21%2c0.00%2c6.18%22%2c%222%2f20%2f2020%2c10%3a50%3a06+PM%22%3a%221.21%2c0.00%2c6.18%22%2c%222%2f20%2f2020%2c10%3a50%3a21+PM%22%3a%221.21%2c0.00%2c6.18%22%2c%222%2f20%2f2020%2c10%3a50%3a36+PM%22%3a%221.21%2c0.00%2c6.13%22%2c%222%2f20%2f2020%2c10%3a50%3a51+PM%22%3a%221.21%2c0.00%2c6.13%22%2c%222%2f20%2f2020%2c10%3a51%3a06+PM%22%3a%221.21%2c0.00%2c6.13%22%2c%222%2f20%2f2020%2c10%3a51%3a21+PM%22%3a%221.21%2c0.00%2c6.13%22%2c%222%2f20%2f2020%2c10%3a51%3a37+PM%22%3a%221.21%2c0.00%2c6.13%22%2c%222%2f20%2f2020%2c10%3a51%3a52+PM%22%3a%221.21%2c0.00%2c6.16%22%2c%222%2f20%2f2020%2c10%3a52%3a07+PM%22%3a%221.21%2c0.00%2c6.18%22%2c%222%2f20%2f2020%2c10%3a52%3a22+PM%22%3a%221.21%2c0.00%2c6.16%22%2c%222%2f20%2f2020%2c10%3a52%3a37+PM%22%3a%221.21%2c0.00%2c6.13%22%2c%222%2f20%2f2020%2c10%3a52%3a52+PM%22%3a%221.21%2c0.00%2c6.18%22%2c%222%2f20%2f2020%2c10%3a53%3a08+PM%22%3a%221.21%2c0.00%2c6.13%22%2c%222%2f20%2f2020%2c10%3a53%3a23+PM%22%3a%221.21%2c0.00%2c6.16%22%2c%222%2f20%2f2020%2c10%3a53%3a38+PM%22%3a%221.21%2c0.00%2c6.16%22%2c%222%2f20%2f2020%2c10%3a53%3a53+PM%22%3a%221.21%2c0.00%2c6.13%22%2c%222%2f20%2f2020%2c10%3a54%3a08+PM%22%3a%221.21%2c0.00%2c6.13%22}
----------------------------------------------------------------------
If interested in using global scope vars, use PropertiesService as demonstrated here:
    https://stackoverflow.com/questions/24721226/how-to-define-global-variable-in-google-apps-script
----------------------------------------------------------------------
--------------------------------------------------------------------*/


/**
 *  Handler for HTTP GET requests sent from an Arduino Mega 2560 + Ethernet/SD Shield using the Spreadsheet API 
**/
function doGet(e) { 
  // Vars to likely be modified in the future:
  var DEBUG_NEW_SHEET_GEN = 0;
  
  //var columnHeaders = ["Date", "Time", "Level (inches)", "Voltage (V)", "Current (mA)"];
  //var columnData = ["levelData", "voltageData", "currentData"];
  var columnHeaders = ["Date", "Time", "Voltage (V)", "Level (inches)", "pH"];
  var columnData = ["voltageData", "levelData", "phData"];
  
  var result = 'OK'; // Assume success to begin
  var createdNewSheet = false;
  
  var id = '1es8jdW2go2Bgrn7Iz5L8bNpIfehzsKK6hWMP0CXh4IQ';  // docs.google.com/spreadsheets/d/{Spreadsheet id}/edit...
  var ss = SpreadsheetApp.openById(id);

  if (e == undefined || e.parameter == undefined) {
    // Log that something went weird along the way...
    result = 'No Parameters';
    Logger.log(result);
    console.log(result);
  }
  else {
    Logger.log(JSON.stringify(e));  // View parameters
    console.log(JSON.stringify(e));
    
    var d = new Date();
    var year = d.getFullYear();
    var month = d.getMonth() + 1;    // getMonth returns # from 0-11
    var sheetName = month + '/' + year;
    
    // Check to see if new Sheet is needed for a new month & create it if so
    var sheet = ss.getSheetByName(sheetName);    //getSheetByName will return a null reference if non-existent
    
    while (DEBUG_NEW_SHEET_GEN && sheet) {
      year += 1;
      sheetName = month + '/' + year;
      sheet = ss.getSheetByName(sheetName);
    }
    
    if (!sheet) {
      ss.insertSheet(sheetName, ss.getNumSheets());    // New Sheet becomes the active Sheet
      sheet = ss.getSheetByName(sheetName);
      Logger.log("New Sheet created: " + sheet.getName());
      console.log("New Sheet created: " + sheet.getName());
      createdNewSheet = true;
      
      // Set up the column headers
      sheet.appendRow(columnHeaders);
      sheet.setColumnWidths(1, columnHeaders.length, 130);
      var headRange = sheet.getRange(1, 1, 1, columnHeaders.length);
      headRange.setHorizontalAlignment("center");
      headRange.setFontSize(11);
      headRange.setBorder(null, null, true, null, null, null);    // Set bottom border
    }
    
    var obj = JSON.parse(e.parameter["json"]);
    //var obj = JSON.parse(stripQuotes(e.parameter[0]));
    Logger.log(stripQuotes(e.parameter["json"]));
    console.log(stripQuotes(e.parameter["json"]));
    
    // var newRow = sheet.getLastRow() + 1;
    // var rowData = [];

    Object.keys(obj).forEach ( function(key) {
      var value = obj[key];
      var date = key.split(',')[0];
      var time = key.split(',')[1];
      var volts = parseFloat(value.split(',')[0]);
      var level = parseFloat(value.split(',')[1]);
      var ph = parseFloat(value.split(',')[2]);
      
      var newRow = sheet.getLastRow() + 1;
      var rowData = [];
      rowData[0] = date;      
      rowData[1] = time;
      rowData[2] = volts;
      rowData[3] = level;
      rowData[4] = ph;
      
      // Write new row below
      var newRange = sheet.getRange(newRow, 1, 1, rowData.length);
      newRange.setValues([rowData]);
      newRange.setHorizontalAlignment("center");
    });

    var hour = d.getHours();
    var ampm = (hour < 12) ? ' AM' : ' PM';
    if (hour == 0) hour = 12;
    else if (hour > 12) hour -= 12;
    
    var date = month + '/' + d.getDate() + '/' + year;
    var lastDate = String(PropertiesService.getScriptProperties().getProperty('today'));
    
    // Logger.log("Same date? --> " + (String(date).localeCompare(lastDate) == 0));
    //  console.log("Same date? --> " + (String(date).localeCompare(lastDate) == 0));
    // Logger.log("date: " + date + "  lastDate: " + lastDate);
    //  console.log("date: " + date + "  lastDate: " + lastDate);
    
    if (String(date).localeCompare(lastDate) != 0) {
      publishDailyResults(lastDate, String(date), sheet, ss, createdNewSheet);
    }
    
    /*
    rowData[0] = date;                                                        // Date in column A
    rowData[1] = hour + ':' + d.getMinutes() + ':' + d.getSeconds() + ampm;   // Time in column B
    
    for (var param in e.parameter) {
      var value = stripQuotes(e.parameter[param]);
      switch (param) {
        case columnData[0]:    // Parameter
          rowData[2] = value;  // Value in column C
          break;
        case columnData[1]:
          rowData[3] = value;  // Value in column D
          break;
        case columnData[2]:
          rowData[4] = value;  // Value in column E
          break;
        default:
          result = "Unsupported Parameter";
          rowData[2] = 'ERR';
          rowData[3] = 'ERR';
          rowData[4] = 'ERR';
      }
    }
    Logger.log(JSON.stringify(rowData));
    console.log(JSON.stringify(rowData));

    // Write new row below
    var newRange = sheet.getRange(newRow, 1, 1, rowData.length);
    newRange.setValues([rowData]);
    newRange.setHorizontalAlignment("center");
    */
  }

  // Return result of operation
  return ContentService.createTextOutput(result);
}


function publishDailyResults(oldDate, newDate, curSheet, ss, createdNewSheet) {
  var resultSheet = ss.getSheetByName('Flow&pH');
  if (!resultSheet) {
    Logger.log("[publishDailyResults] Result Sheet is NULL:  Flow&pH"); 
    console.log("[publishDailyResults] Result Sheet is NULL:  Flow&pH"); 
    return;
  }

  var todayRow =  PropertiesService.getScriptProperties().getProperty('todayFirstRow');
  var newTodayRow = (+todayRow) | 0;   // = parseInt(todayRow);
  
  var pHLow  = 14.0;
  var pHHigh = 0.00;
  var galPerDay = 0.00;
  var secondsObserved = 0;
  var secondsInDay = 86400;  // (86400 / 15) == 5760 intervals
  var galPerCubicFt = 7.480543;
  var Q = 0.0;
  var K = 0.338;
  var n = 1.9;
  var loops = 0;
  
  var curRow = curSheet.getRange(newTodayRow, 1, 1, 5);
  var curRowVals = curRow.getValues();
  var curRowDateStr = String((curRowVals[0][0]).getMonth()+1) + '/' + (curRowVals[0][0]).getDate() + '/' + (curRowVals[0][0]).getFullYear();
  
  // Logger.log(curRowVals);
  //    console.log(curRowVals);
  // Logger.log("curRowDateStr: " + curRowDateStr);
  //    console.log("curRowDateStr: " + curRowDateStr);
  
  while ((String(oldDate).localeCompare(curRowDateStr) == 0)) { // && (secondsObserved <= secondsInDay)) {
   // Computations here ...
    if ((curRowVals[0][4] * 1.0) < pHLow)
      pHLow = (curRowVals[0][4] * 1.0);
    if ((curRowVals[0][4] * 1.0) > pHHigh)
      pHHigh = (curRowVals[0][4] * 1.0);
    
    var levelInFt = curRowVals[0][3] / 12.0;
    Q = K * (Math.pow(levelInFt, n));  // Q represents flow in cubic feet per second
    var galPerInterval = (Q * galPerCubicFt) * 15;  // gallons flowing over 15 second time interval
    galPerDay += galPerInterval;
    
    secondsObserved += 15;
    newTodayRow = (newTodayRow + 1) | 0;
    curRow = curSheet.getRange(newTodayRow, 1, 1, 5);
    curRowVals = curRow.getValues();
    try {
      curRowDateStr = String((curRowVals[0][0]).getMonth()+1) + '/' + (curRowVals[0][0]).getDate() + '/' + (curRowVals[0][0]).getFullYear();
    }
    catch(err) {
      break; 
    }
    loops += 1;
  }
  
  Logger.log("publishDailyResults loops: " + loops);
      console.log("publishDailyResults loops: " + loops);
  
  // Write new row into results sheet...
  var newRow = resultSheet.getLastRow() + 1;
  var rowData = [];
  rowData[0] = oldDate;
  rowData[1] = galPerDay;
  rowData[2] = pHHigh;
  rowData[3] = pHLow;
  
  var newRange = resultSheet.getRange(newRow, 1, 1, rowData.length);
  newRange.setValues([rowData]);
  newRange.setHorizontalAlignment("center");
  
  Logger.log("publishDailyResults rowData: " + rowData);
      console.log("publishDailyResults rowData: " + rowData);
  
  PropertiesService.getScriptProperties().setProperty('today', newDate);
  if (createdNewSheet)
    PropertiesService.getScriptProperties().setProperty('todayFirstRow', 2); 
  else
    PropertiesService.getScriptProperties().setProperty('todayFirstRow', newTodayRow | 0); 
}


/**
 *  Remove leading and trailing single or double quotes
**/
function stripQuotes(value) {
  return value.replace(/^["']|['"]$/g, "");
}