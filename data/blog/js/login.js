$(function() {
   	$("#login-form").submit(function (e) {
	    
    	$(this).find(".alert").empty().addClass('hidden') ;
    	e.preventDefault();
    	
    	var frm = this ;
       	$.ajax({
			type: "POST",
		    url: "user/login/",
		    dataType: "json",
		    data: $(this).serialize(), // serializes the form's elements.
			success: function(data)	{
				if ( data.hasOwnProperty("error") && data.error ) 
						$(frm).find('.alert').addClass('alert-error').html(data.error).toggleClass('hidden') ;
       		   	else {
					location.reload(false) ;
       		   	}
            }
        }) ;
    });
    
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

