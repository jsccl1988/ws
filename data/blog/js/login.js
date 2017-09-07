$(function() {
   	$("#login").click(function (e) {
	    
    	e.preventDefault();

		var msg = $('<div></div>').load('/user/login/', 
				function(response, status, xhr) {
					var dialog = new BootstrapDialog({ 
						title: 'Sign In',
						message: msg
					});
							
					dialog.realize() ;
					dialog.open() ;
        
					var form = msg.find('form') ;
					form.submit(function(e) {
						$.ajax({
							type: "POST",
							dataType: "json",
							url: '/user/login/',
							data: form.serialize(), // serializes the form's elements.
							success: function(data)
							{
								if ( data.success ) {
									dialog.close() ;
									location.reload(false) ;
									
								}
								else {
									form.html(data.content) ;
								}
							} 
						});
								
						e.preventDefault() ;
					}) ;
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

