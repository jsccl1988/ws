(function ( $ ) {
 
    var current_page = 1 ;
  	var results_per_view = 10 ;

    var item ;
    
 
    $.fn.tableview = function(options) {
    	var item = this ;
    	
    	function onNewRecord() 	{
			item.formModal({url: options.addUrl, title: 'New Record'}) ;
		} ;
				
		function onEditRecord() 	{
			var id = $(this).closest('tr').data('id') ;
			item.formModal({url: options.updateUrl, title: 'Edit Record', data: {id: id}, onSuccess: function() { reload() ; }}) ;
		} ;
				
		function onDeleteRecord() {
			var id = $(this).closest('tr').data('id') ;
			 $.ajax({
	    		url: options['deleteUrl'],
		   	    type: 'POST',
		   	    dataType: 'json',
	    		data: { 'id': id },
	    		cache: false,
	    		success: function(data, textStatus, jqXHR)	{
					reload() ;
				},
				error: function(jqXHR, textStatus, errorThrown)	{
       			    // Handle errors here
       			    console.log('ERRORS: ' + textStatus);
       			    // STOP LOADING SPINNER
       			}
	    	}) ;
		}
				
    	function reload() {
  			$(item).load(options['listUrl'], $.param({page: current_page, total: results_per_view}), function(response, status, xhr) {
						
			// add handler for pager links
			$(item).find('#pager li a').click(function(e) {
				e.preventDefault() ;
				current_page = $(this).data('page') ;
				reload() ;
			}) ;
						
			// select correct button in results per view group
			
			var rpv_buttons = $(item).find('#results-per-page input') ;
				
			$(rpv_buttons).each(function(item, value) {
				if ( $(value).val() == results_per_view ) $(value).parent().addClass("active") ;
				else $(value).parent().removeClass("active") ;
			}) ;
						
			// assure that the current page is within range
			var num_pages = $(item).find('ul').data('total-pages') ;
			if ( current_page > num_pages ) current_page = 1 ;
						
			// handle results per view change

			$(rpv_buttons).change(function(){
				results_per_view = $(this).val() ;
				reload() ;
			})
						
			// handle new record action
						
			$(item).find('#new-record').click(onNewRecord) ;
		  				
			// handle detete record
			$(item).find('#data-grid #delete-button').click(onDeleteRecord) ;
		  				
			// handle edit record
			$(item).find('#data-grid #edit-button').click(onEditRecord) ;
			}) ;
		}
    
		reload() ;
					

        return this;
    };
 
}( jQuery ));
