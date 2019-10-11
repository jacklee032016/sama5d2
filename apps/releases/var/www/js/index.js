//** Api url address used in ajax functions **//
//var apiUrl = "http://angelicaq.api" //dev
//var apiUrl = "http://" + window.location.hostname // + "/api" //dev and prod
//var apiUrl = "http://192.168.168.100" //dev and prod

//var apiUrl = "http://127.0.0.1/api" //dev


var apiUrl = "http://" + window.location.hostname + ":8888" //prod 

// device params info
var device = null; 

// Code executed after body loaded (insert after body tag)
var loading = $('#loading-image');
loading.hide();
$(document).ajaxStart(function () {
    loading.show();
});
$(document).ajaxStop(function () {
    loading.hide();
});

//** script events listener **/
//delete a script
$(document).on("click", ".btnRemoveScript", function() {
    var row       = this.closest('tr');
    var type      = $(row).find('td:eq(0)').html();
    var name      = $(row).find('td:eq(1)').html();
    var window    = $(row).find('td:eq(2)').html();
    var week      = $(row).find('td:eq(3)').html();
    var startTime = $(row).find('td:eq(4)').html();

    if (type == "Repeat")
        deleteScript(type, "repeat", -1, 6, "23:59", row);
    else    
        deleteScript(type, name, getWindowCode(window), getCrontWeekCodeFullName(week), startTime, row);
});

//script update modal view
$(document).on("click", ".btnUpdateScript", function() {
    var row       = this.closest('tr');
    var type      = $(row).find('td:eq(0)').html();
    var plName    = $(row).find('td:eq(1)').html();
    var window    = $(row).find('td:eq(2)').html();
    var week      = $(row).find('td:eq(3)').html();
    var startTime = $(row).find('td:eq(4)').html();

    $("#inHiddenEntry").val(type);
    $("#laSlPlaylist").html("Selected Playlist:");
    $('#laPlaylist').html(plName);
    $('#startTime').val(startTime);
    $('#origStartTime').val(startTime);
    $('#weekDay').val(getCrontWeekCodeFullName(week));
    $('#origWeekDay').val(getCrontWeekCodeFullName(week));
    $('#slWindow').val(getWindowCode(window));
    $('#origWindow').val(getWindowCode(window));
    
     toggleModal("Update");
    if (type == "Playlist")
    {
        $("#divWindow").show();
    }
    else
    {
        $("#divWindow").hide();
    }
});


//** playlist events listener **/
//delete a playlist
$(document).on("click", "#btnDeletePlaylist", function() {
    var playlistName = this.closest('.plwrap').id;
    deletePlaylist(playlistName);
});

//play a playlist file 
$(document).on("click", ".btnMediaPlay", function() {
    var media = decodeEntities($(this.closest(".list-group-item")).find("span").html());
    playMedia('play', media);
});

//remove a playlist file 
$(document).on("click", ".btnRemoveMedia", function() {
    var plName      = $.trim($("span#spName").html());
    var description = $.trim($("span#spDescription").html());
    $(this.closest(".list-group-item")).addClass("liRemove");
    var fileList = getFileList();

    updatePlaylist(plName, plName, description, fileList, "delete");
});

//playlist update modal view
$(document).on("click", ".btnUpdatePlaylist", function() {
    var playlistName = this.closest('.plwrap').id;
    var files = getFileList(playlistName);
    $(".ulNewList").remove();
    $("#inpName").val(playlistName);
    $("#inpOrigName").val(playlistName);
    toggleModal("Update");
    
    var view = '<ul class="list-group ulNewList">'; 
    $.each(files, function(i, file) {
        view += viewModalFileUl(file);
    });
    view += '</ul>';
    $("div#divAnchor").html(view);
});
//remove a playlist file in modal view (dialog)
$(document).on("click", ".btnModalRemoveMedia", function() {
    this.closest('.plList').remove();
});
//stop a playlist
$(document).on("click", ".btnPlaylistStop", function() {
    var playlistName = this.closest('.plwrap').id;
    playMedia('stop', playlistName);
});
//play a playlist
$(document).on("click", ".btnPlaylistPlay", function() {
    var playlistName = this.closest('.plwrap').id;
    playMedia('play', playlistName);
});
//pause a playlist
$(document).on("click", ".btnPlaylistPause", function() {
    var playlistName = this.closest('.plwrap').id;
    playMedia('pause', playlistName);
});
//resume a playlist
$(document).on("click", ".btnPlaylistResume", function() {
    var playlistName = this.closest('.plwrap').id;
    playMedia('resume', playlistName);
});
//add video file to the playlist button click
$(document).on("click", ".btnAddFile", function(e) {
    var fileName    = $('#selectFiles option:selected').text();
    var plName      = $.trim($("span#spName").html());
    var description = $.trim($("span#spDescription").html());
    var fileList    = getFileList();

    if (fileName != 'NONE')
    {
        var file = JSON.parse($('#selectFiles').val());
        fileList.push({name:file.name, duration:file.duration});
        
        updatePlaylist(plName, plName, description, fileList, "addVideo");
    }
    e.preventDefault();
    return false;
});
//add image file with duration to playlist
$(document).on("click", ".btnAddImage", function(e) {
    var image    = $('#selectImageFiles').val();
    var duration = $("#inImageDuration").val();
    
    if (duration == "" || duration == "00:00:00")
    {
        alert("Image duration can not be empty or 00:00:00");
        return false;
    }

    var plName      = $.trim($("span#spName").html());
    var description = $.trim($("span#spDescription").html());
    var fileList    = getFileList();

    if (image != 'NONE')
    {
        duration = HMStoSeconds(duration);
        var file = JSON.parse($('#selectImageFiles').val());
        fileList.push({name:file.name, duration:duration});
        
        updatePlaylist(plName, plName, description, fileList, "addImage");
    }
    e.preventDefault();
    return false;
});
//add URL video stream to playlist
$(document).on("click", ".btnAddUrl", function(e) {
    var url = $('#inUrl').val();
    var pattern = /^((http|https|rtsp|rtp|udp|wmsp|rtmp):\/\/)/;

    if(!pattern.test(url))
    {
        alert('url protocols: http://, https://, rtsp://, rtp://, udp://, wmsp:// or rtmp://');
        return false;
    }

    var plName      = $.trim($("span#spName").html());
    var description = $.trim($("span#spDescription").html());
    var fileList    = getFileList();

    fileList.push({name:url, duration:0});
    
    updatePlaylist(plName, plName, description, fileList, "addUrl");
    e.preventDefault();
    return false;
});

//** playlist event listener **/

$.cookie.json = true;

//check if there is a token, if not call login page  
var token = $.cookie("token");
/*if (!token)
{
    loadContent('userlogin.html', 'Please Login');
}
else
{*/
    $('#blink').css('visibility', 'visible').css( 'display','block');
//    submit_main('info.html');
//}

getTypeInfo();

//*** functions *****
//playlist get media names from ul list
function getFileList()
{
    var files = new Array();
    $("#divAnchor").find('ul.list-group-sortable li:not(".liRemove")').each(function(){     
        fileName     = decodeEntities($(this).find('span').html());
        fileDuration = $(this).find('input[type="hidden"]').val();
        fileType     = checkFileTypeOrUrl(fileName);
        if (fileType == "URL")
        {
            temp = {name:fileName, duration:0};
        }
        else //IMAGE
        {
            temp = {name:fileName, duration:fileDuration};
        }
            
        files.push(temp);
    });
    
    return files;
}

function decodeEntities(encodedString) {
    var textArea = document.createElement('textarea');
    textArea.innerHTML = encodedString;
    return textArea.value;
}

//Delay for a number of milliseconds
function sleep(delay)
{
    var start = new Date().getTime();
    while (new Date().getTime() < start + delay);
}

function checkDevice()
{
    device = $.cookie("device");
    if (!device)
        getParam();
}

function submit_main(page, message = null)
{
    var token = $.cookie("token");
/*    if (!token)
        loadContent('userlogin.html', 'Please Login');
    else{*/
        loadContent(page, message);
        /*//check if token is valid
        $.ajax({
            type: 'GET',
            url: apiUrl + "/device/param",
            headers: {"Authorization": token},
            async: false,
            success: function(response){
                if (response.status == "success")
                {
                    //device = $.cookie("device");
                    device = response.data
                    loadContent(page, message);
                }
                else
                {
                    $('#message').css("display", "block");
                    $('#message').html("Unable to communicate correctly");
                    $("#content").html("");
                }
            },
            error: function (jqXHR, status) {
                if (jqXHR.status == 401) //token not valid or expired
                {
                    loadContent('userlogin.html', 'Please login');
                }
            }
        })*/
    //}
}

function loadContent(page, message = null)
{
    $("#content").load('include/' + page);

    //bold menu items           
    $('.bolder').each(function(i, obj) {
        $(obj).attr("class","");
    });
    var nav_link = page.replace(".html", "");
    nav_link = "#nav_" + nav_link;
    $(nav_link).attr("class","bolder");

    //set message
    if (message)
    {
        document.getElementById('message').style.display = "block";
        document.getElementById('message').innerHTML = message;
    }       
    else
    {
        document.getElementById('message').style.display = "none";
        document.getElementById('message').innerHTML = null;    
    }
}

function needReboot(oldDevice, device)
{
    //check DipOn
    if ( device["isDipOn"] && ( oldDevice["isDipOn"] != device["isDipOn"] ) ||
         device["isDhcp"] && ( oldDevice["isDhcp"] != device["isDhcp"] ) ||
         device["ip"] && ( oldDevice["ip"] != device["ip"] ) ||
         device["mask"] && ( oldDevice["mask"] != device["mask"] ) ||
         device["gateway"] && ( oldDevice["gateway"] != device["gateway"] ) ||
         device["dns1"] && ( oldDevice["dns1"] != device["dns1"] ) ||
         device["dns2"] && ( oldDevice["dns2"] != device["dns2"] )
       )
        return true;

    return false;
}

//*** ajax functions ****/
function signOut()
{
    var token = $.cookie("token");
    
    $.ajax({
        type: 'POST',
        url: apiUrl + "/auth/signout",
        headers: {"Authorization": token},
        success: function(response) {
            $.removeCookie('token');
            $.removeCookie('device');
            $('#blink').css('visibility', 'hidden').css('display','none');
            $('#message').css('display','block').html("Logged out");
            $("#content").html("");
        },
        error: function (jqXHR, status) {
            if (jqXHR.status == 401) //token not valid or expired
            {
                loadContent('userlogin.html', 'Please login');
            }
        }
    });
}



//blink functionality
function blink(value)
{
    var token = $.cookie("token");
    $.ajax({
        method: "PUT",
        url: apiUrl + "/device/blink",
        headers: {"Authorization": token},
        data: { "blink" : value },
        success: function(response) {
            if (response.data.blink == true) 
            {
                if (!$("#blink button").hasClass("blink-button"))
                {
                    $("#blink button").attr("onClick","javascript:blink('false');");
                    $("#blink button").addClass("blink-button");
                    $("#blink button").attr("value","Turn OFF");
                    return;
                }
            }
            $("#blink button").attr("onClick","javascript:blink('true');");
            $("#blink button").removeClass("blink-button");
            $("#blink button").attr("value","Blink LED");
        },
        error: function() {
            //if error, initial state 
            $("#blink button").attr("onClick","javascript:blink('true');");
            $("#blink button").removeClass("blink-button");
            $("#blink button").attr("value","Blink LED");
        }
    });
}


// get device parameters
function getParam()
{
    var token = $.cookie("token");
    $.ajax({
        type: 'GET',
        url: apiUrl + "/device/param",
        headers: {"Authorization": token},
        async: false,
        success: function(response){
            if (response.status == "success")
            {
                tmp = response.data;
                delete tmp.others;
                delete tmp.security.get_id;
                delete tmp.security.set_key;
                delete tmp.IR;
                $.cookie("device", tmp);
                device = tmp;
            }
            else
            {
                $('#message').css("display", "block");
                $('#message').html("Unable to communicate correctly");
                $("#content").html("");
            }
        },
        error: function (jqXHR, status) {
            if (jqXHR.status == 401) //token not valid or expired
            {
                loadContent('userlogin.html', 'Please login');
            }
        }
    });
}

//get footer info
function getTypeInfo()
{
    $.ajax({
        type: 'GET',
        url: apiUrl + "/device/param",
        success: function(response){
            if (response.status == "success")
            {
                tmp = response.data;
                delete tmp.others;
                delete tmp.security.get_id;
                delete tmp.security.set_key;
                delete tmp.IR;
                $.cookie("device", tmp);
                isTx    = response.data.system.isTx;
                version = response.data.system.ver;
                built   = response.data.system.built;
                submit_main('info.html');
                if (isTx)
                    $("#footer").html("&copy; MuxLab Inc. 500774 TX HDMI 2.0/ST2110 over IP (Version: "+version+'; Built: '+built+")");
                else 
                    $("#footer").html("&copy; MuxLab Inc. 500774 RX HDMI 2.0/ST2110 over IP (Version: "+version+'; Built: '+built+")");
            
                if (isTx)
                    $("#id_mainProductName").text("500774-TX HDMI 2.0/ST2110 over IP Uncompressed Gateway Converter TX");
                else
                    $("#id_mainProductName").text("500774-RX HDMI 2.0/ST2110 over IP Uncompressed Gateway Converter RX");
            }
        },
    });
}

function setDeviceCookie()
{
    var token = $.cookie("token");
    $.ajax({
        type: 'GET',
        url: apiUrl + "/device/param",
        headers: {"Authorization": token},
        async: false,
        success: function(response){
            if (response.status == "success")
            {
                tmp = response.data;
                delete tmp.others;
                delete tmp.security.get_id;
                delete tmp.security.set_key;
                delete tmp.anc;
                delete tmp.IR;
                $.cookie("device", tmp);
                $('#blink').css('visibility', 'visible').css( 'display','block');
                loadContent("info.html", "Logged in");
            }
            else
            {
                $('#message').css("display", "block");
                $('#message').html("Unable to communicate correctly");
                $("#content").html("");
            }
        },
        error: function (jqXHR, status) {
            if (jqXHR.status == 401) //token not valid or expired
            {
                loadContent('userlogin.html', 'Please login');
            }
        }
    });
}

//set device parameters
function setParam(device, page)
{
    var token     = $.cookie("token");
    var oldDevice = $.cookie("device");
    $.ajax({
        type: 'POST',
        url: apiUrl + "/device/param",
        headers: {"Authorization": token},
        data: device,
        async: false,
        success: function(response){
            if (response.status == "success")
            {
                if (needReboot(oldDevice, device))
                {
                    $('#message').css("display", "block");
                    $('#message').html("Saving&nbsp;&nbsp;&nbsp;");
                    $("#content").html("");
                    var dots = 0;
                    var threeDots = window.setInterval( function() 
                    {
                        if ( dots == 1 )
                            $('#message').html("Saving.&nbsp;&nbsp;");
                        else if ( dots == 2 )
                            $('#message').html("Saving..&nbsp;");
                        else if (dots == 3)
                            $('#message').html("Saving...");
                        else{
                            $('#message').html("Saving&nbsp;&nbsp;&nbsp;");
                            dots = 0;
                        }
                        
                        dots++;
                    }, 200);
                    
                    window.setTimeout( function()
                    {
                        window.clearInterval(threeDots);
                        loadContent(page);
                        if (page == 'decoder.html')
                            completeDecoderPage();
                        else if (page == 'network.html')
                            completeNetworkPage();
                        
                    }, 30000);
                }
                else
                {
                    $('#message').css("display", "block");
                    $('#message').html("Settings saved");
                    if (page == 'decoder.html')
                        completeDecoderPage();
                    else if (page == 'network.html')
                        completeNetworkPage();
                }
            }
            else
            {
                $('#message').css("display", "block");
                $('#message').html("Save Error");
            }
        },
        error: function (jqXHR, status) {
            if (jqXHR.status == 401) //token not valid or expired
            {
                loadContent('userlogin.html', 'Please login');
            }
        }
    });
}
