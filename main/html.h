const char* html_page = 
    "<!DOCTYPE html>\n"
    "<head>\n"
    "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
    "    <meta http-equiv='refresh' content='1'>\n"  // Refresh every 1 second
    "    <style>\n"
    "        * { margin: 0; padding: 0; box-sizing: border-box; }\n"
    "        body, html { height: 100%; display: flex; justify-content: center; align-items: center; background-color: #f3f4f6; color: #333; }\n"
    "        .timer { font-size: 3em; font-weight: bold; color: #4caf50; }\n"
    "    </style>\n"
    "</head>\n"
    "<body>\n"
    "    <div class=\"timer\">%d seconds</div>\n"  // Counter display
    "</body>\n"
    "</html>";
    
const char* html_oneline_page = "<!DOCTYPE html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><meta http-equiv='refresh' content='1'><style>*{margin:0;padding:0;box-sizing:border-box;}body,html{height:100%;display:flex;justify-content:center;align-items:center;background-color:#f3f4f6;color:#333;}.timer{font-size:3em;font-weight:bold;color:#4caf50;}</style></head><body><div class=\"timer\">%d seconds</div></body></html>";
