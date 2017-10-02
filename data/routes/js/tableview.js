(function ( $ ) {
 
    var current_page = 1 ;
  	var results_per_view = 10 ;

    var item ;
    
 
    $.fn.tableview = function(options) {
    	var item = this ;
    	
    	var table = $('<div id="grid"></div>').appendTo(item) ;
    	$('<div id="create-form"></div>').appendTo(item).formModal('create', {title: 'New Record'}) ;
    	$('<div id="edit-form"></div>').appendTo(item).formModal('create', {title: 'Edit Record'}) ;
    	
    	function onNewRecord() 	{
    		item.find('#create-form').formModal('show', {url: options.addUrl, onSuccess: function() { reload() ; }}) ;
		} ;
				
		function onEditRecord() 	{
			var id = $(this).closest('tr').data('id') ;
			item.find('#edit-form').formModal('show', {url: options.updateUrl,  data: {id: id}, onSuccess: function() { reload() ; }}) ;
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
    		table.empty() ;
  			table.load(options['listUrl'], $.param({page: current_page, total: results_per_view}), function(response, status, xhr) {
			
				// add handler for pager links
				$(table).find('#pager li a').click(function(e) {
					e.preventDefault() ;
					current_page = $(this).data('page') ;
					reload() ;
				}) ;
						
				// select correct button in results per view group
			
				var rpv_buttons = $(table).find('#results-per-page input') ;
				
				$(rpv_buttons).each(function(item, value) {
					if ( $(value).val() == results_per_view ) $(value).parent().addClass("active") ;
					else $(value).parent().removeClass("active") ;
				}) ;
						
				// assure that the current page is within range
				var num_pages = $(table).find('ul').data('total-pages') ;
				if ( current_page > num_pages ) current_page = 1 ;
						
				// handle results per view change

				$(rpv_buttons).change(function(){
					results_per_view = $(this).val() ;
					reload() ;
				})
						
				// handle new record action
						
				$(table).find('#new-record').click(onNewRecord) ;
		  				
				// handle detete record
				$(table).find('#data-grid #delete-button').click(onDeleteRecord) ;
		  				
				// handle edit record
				$(table).find('#data-grid #edit-button').click(onEditRecord) ;
			}) ;
			
			$(item).append(table) ;
		}
    
		reload() ;
					

        return this;
    };
 
}( jQuery ));
