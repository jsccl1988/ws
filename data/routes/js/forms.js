// helper for auto-handling of form submit and form dialogs

(function($) {
	$.fn.form = function(params) { 
//	params = $.extend( {minlength: 0, maxlength: 99999}, params);
	
		// traverse all nodes
		this.each(function() {
			var that = $(this);
			var form = that.find('form') ;
			
			
			function clearErrors() {
				$('#global-errors', form).remove() ;
				
				$('.form-group' , form).each(function(index) {
					$(this).removeClass("has-error has-feedback") ;
					$('#errors', this).remove() ;
				}) ;
				
			
			}
			
			form.submit(function(e) {
				 var formData = new FormData(this);
				$.ajax({
					type: "POST",
					dataType: "json",
					url: params.url,
					processData: false,
				    contentType: false,
//					data: form.serialize(), // serializes the form's elements.
					data: formData,
					success: function(data)	{
						if ( data.success ) 
							params.onSuccess() ;
						else {
							if ( data.hasOwnProperty("content") ) // server has send the whole form
								form.html(data.content) ;
							else { // render errors	only
								clearErrors() ;
								if ( data.errors['global-errors'].length > 0 ) {
									var et = $('<div id="global-errors" class="alert alert-danger"><a class="close" data-dismiss="alert">&times;</a></div>') ;
									et.prependTo(form);
									for( item in data.errors['global-errors'] ) {
										et.append('<p>' + data.errors['global-errors'][item] + '</p>') ;
    	    						}
								}
								for( key in data.errors['field-errors'] ) {
									var error_list = data.errors['field-errors'][key] ;
									if ( error_list.length > 0 ) {
										var et = $('.form-group [name=' + key + ']', form).parent() ;
										et.addClass("has-error has-feedback") ;
										var error_block = $('<div id="errors"></div>') ;
	
										$('<span class="glyphicon glyphicon-remove form-control-feedback"></span>').appendTo(error_block) ;
										var msg_block = $('<span class="help-block"></span>') ;
										for( item in error_list ) {
											msg_block.append('<p>' + error_list[item] + '</p>') ;
    		    						}
    		    						msg_block.appendTo(error_block) ;
									    error_block.appendTo(et) ;
									}
								}
							}
							
						}
					} 
				});
								
				e.preventDefault() ;
			}) ;
			
	
		});
		
		return this ;
	};
	
	$.fn.formModal = function(params) { 
		var defaults = { onSuccess: function () {} } ;
		var params = $.extend( {}, defaults, params );
		this.each(function() {
			var that = $(this);
			
			that.click(function (e) {
	    
		    	e.preventDefault();
		    
		    	var msg = $('<div></div>').load(params.url, 
				function(response, status, xhr) {
					msg.find(':file').filestyle({btnClass: "btn-primary"}) ;
					var dialog = new BootstrapDialog({ 
						title: params.title,
						message: msg
					});
					
							
					dialog.realize() ;
					dialog.open() ;
        
					var form = msg.form({ 
						url: params.url,
						onSuccess: function() {
							dialog.close() ;
							params.onSuccess() ;
						}
					}) ; 
					
				}) ;
			}) ;	
		});
		
		return this ;
	};
	

})(jQuery);
