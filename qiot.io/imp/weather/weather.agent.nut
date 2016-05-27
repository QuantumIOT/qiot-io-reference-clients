device.on("qiot.io", function(blob) {
    local url = "http://54.175.80.78:13000/resources/1/l/t/" + imp.configparams.deviceid
    local headers = { "Content-Type": "application/json", "Authorization": "QIOT cmVzdC1hcGktdXNlcjowNGE4ZTBkOS1jNjEwLTQzMDgtYjU2OS01YmJlN2M4OTAzNjA=" }

    local headersJson = http.jsonencode(headers)
    local bodyJson = http.jsonencode(blob)

    http.post(url, headers, bodyJson).sendasync(function (response) {
        server.log("----")
        server.log("SEND TO QIOT.IO")
        server.log("qiot.io send: " + url + " " + headersJson + " " + bodyJson)        
        server.log("qiot.io recv: [" + response.statuscode + "] - " + response.body)        
    }) 
})
