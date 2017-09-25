$(function() {

	$("#login").formModal({url: 'user/login', title: 'Sign In'}) ;


    $("#logout").click(function(e) {
	    e.preventDefault() ;
		$.ajax({
		    url: "user/logout/",
		    type: "POST",
			success: function(data)	{
       		   	location.reload(false) ;
            }
        }) ;
	}) ;
});

