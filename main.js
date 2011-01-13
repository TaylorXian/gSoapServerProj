var soap_req = "";
$(document).ready(function() {
    $('#btn').click(function() {
        $('#lblInfo').text("发送请求... ...");
        jQuery.ajax({
            type: "POST", //"GET",
            url: document.URL,
            contentType: "application/*",
            data: soap_req.toString(),
            success: function(result) {
                alert(soap_req);
                alert(result);
                $(result).find('result').each(function() {
                    alert($(this).text());
                });
            },
            error: function(result) {
                $('#lblInfo').text($(result.responseXML).find('faultstring').text());
            }
        });
    });
});
$(document).ready(function() {
    $('#btnGetRequestSoapData').click(function() {
        $('#lblInfo').text("发送请求...");
        jQuery.ajax({
            type: "GET", //"POST", 
            url: document.URL + "getsoapdata",
            contentType: "application/soap",
            success: function(result) {
                soap_req = result;
                soap_req = soap_req.replace('<key></key>', '<key>' + $('#txtKey').attr("value") + '</key>');
                soap_req = soap_req.replace('<value></value>', '<value>' + $('#txtVal').attr("value") + '</value>');
                $('#btn').click();
            }
        });
    });
});
$(document).ready(function() {
    $('#btnClose').click(function() {
        $('#dialog').hide('slow');
    });
    $('#btnModify').click(function() {
        //$('#btnClose').click();
        $('#btnGetRequestSoapData').click();
    });
});
$(document).ready(function() {
    $('#btnGetTable').click(function() {
        $(document.getElementById('config').rows).each(function() {
            input = $(this).append('<td><input type="button" value="modify" id="' + $(this).index() + '" /></td>').find('input');
            input.click(function() {
                t = (document.body.clientHeight - $('#dialog').height()) / 2;
                l = (document.body.clientWidth - $('#dialog').width()) / 2;
                if (t < 0) t = 0;
                if (l < 0) l = 0;
                k = $(document.getElementById('config').rows).get(this.id).cells[0];
                v = $(document.getElementById('config').rows).get(this.id).cells[2];
                $('#dialog').css({ position: "fixed", top: t, left: l });
                $('#dialog').show('slow', function() {
                    $('#txtKey').attr("value", $(k).text());
                    $('#txtVal').attr("value", $(v).text()).focus();
                });
                $('#lblInfo').text("");
            });
            $(this).mouseover(function(event) {
                e = $(this).css('background-color', 'Aqua');
                e = event.toString();
            })
            .mouseout(function(event) {
                e = event.toString();
                $(this).css('background-color', 'transparent');
            });
        });
    });
    $('#btnGetTable').click();
});
