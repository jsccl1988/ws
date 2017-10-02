// helper for auto-handling of form submit and form dialogs

(function($) {
	$.fn.form = function(params) { 
	var defaults = { data: {}, onSuccess: function() {} } ;
		var params = $.extend( {}, defaults, params );
	
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
				var form_data = new FormData(this);
				for ( var key in params.data ) {
    				form_data.append(key, params.data[key]);
				}	

				$.ajax({
					type: "POST",
					dataType: "json",
					url: params.url,
					processData: false,
				    contentType: false,
//					data: form.serialize(), // serializes the form's elements.
					data: form_data,
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
	
	var bs_modal_html='<div class="modal-dialog"><div class="modal-content"><div class="modal-header"><button type="button" class="close" data-dismiss="modal">&times;</button><h4 class="modal-title"></h4></div><div class="modal-body"></div></div></div>' ;
	
	$.fn.formModal = function(cmd, params) { 
	
		var defaults = { onSuccess: function () {}, data: {} } ;
		params = $.extend( {}, defaults, params );
		
		var that = this ;
		
		function load() {
	
			var content = that.find('.modal-body') ;
			 $(content).load(params.url + '?' + $.param(params.data),
				function(response, status, xhr) {
					    
				var form = content.form({ 
					url: params.url,
					data: params.data,
					onSuccess: function() {
						params.onSuccess() ;
						that.modal('hide') ;
					}
				}) ; 
				that.modal('show') ;
			}) ;
		} ;
		
		this.each(function() {
			var that = $(this);
			if ( cmd == 'create' ) {
				that.addClass("modal fade").attr("role", "dialog") ;
				that.append(bs_modal_html) ;
				$('.modal-title', that).html(params.title) ;
			} else if ( cmd == 'show' ) {
				load() ;
			}	
		});
		
		return this ;
	};
	

})(jQuery);
