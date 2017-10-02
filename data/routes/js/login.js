$(function() {

	
	$("#login").click(function(e) {
		e.preventDefault() ;
		$("#login-modal").formModal('show', {url: 'user/login',  onSuccess: function() { location.reload(false); }}) ;
	}) ;
	
	$("#login-modal").formModal('create', {title: 'Sign In'}) ;

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

