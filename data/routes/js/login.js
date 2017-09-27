$(function() {

	$("#login").formModal({url: 'user/login', title: 'Sign In', onSuccess: function() { location.reload(false); }}) ;


    $("#logout").click(function(e) {
	    e.preventDefault() ;
		$.ajax({
		    url: "user/logout/",
		    type: "POST",
			success: function(data)	{
       		   	document.location.href="";
            }
        }) ;
	}) ;
});

