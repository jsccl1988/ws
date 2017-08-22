/* #####################################################################
   #
   #   Project       : Modal Login with jQuery Effects
   #   Author        : Rodrigo Amarante (rodrigockamarante)
   #   Version       : 1.0
   #   Created       : 07/29/2015
   #   Last Change   : 08/04/2015
   #
   ##################################################################### */
   
$(function() {
    
    var formLogin = $('#login-form');

    var animateTime = 150;
    var msgShowTime = 2000;


	$("#logout").click(function(e) {
		$.ajax({
		    url: "/trails/logout.php",
			success: function(data)	
			{
       		   	location.reload(false) ;
            }
        }) ;
        e.preventDefault() ;
	}) ;

    $("form").submit(function (e) {
       	$.ajax({
			type: "POST",
		    url: "/trails/login.php",
		    dataType: "json",
		    data: formLogin.serialize(), // serializes the form's elements.
			success: function(data)	
			{
				if ( data.hasOwnProperty("error") && data.error ) {
	        		  message($('#div-login-msg'), $('#icon-login-msg'), $('#text-login-msg'), 
	        		  "error", "glyphicon-remove", data.error); 	
       		   	}
       		   	else {
	       		   	location.reload(false) ;
       		   		message($('#div-login-msg'), $('#icon-login-msg'), $('#text-login-msg'), 
       		   		"success", "glyphicon-ok", "Login OK");
       		   	}
            }
        }) ;
     		
		e.preventDefault(); // avoid to execute the actual submit of the form.
		return false ;
    });
   
    
    function fade (id, text) {
        id.fadeOut(animateTime, function() {
            $(this).text(text).fadeIn(animateTime);
        });
    }
    
    function message(divTag, iconTag, textTag, divClass, iconClass, msgText) {
        var msgOld = divTag.text();
        fade(textTag, msgText);
        divTag.addClass(divClass);
        iconTag.removeClass("glyphicon-chevron-right");
        iconTag.addClass(iconClass + " " + divClass);
        setTimeout(function() {
            fade(textTag, msgOld);
            divTag.removeClass(divClass);
            iconTag.addClass("glyphicon-chevron-right");
            iconTag.removeClass(iconClass + " " + divClass);
  		}, msgShowTime);
    }
});
