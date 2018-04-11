(function ( $ ) {
 
    var current_page = 1 ;
  	var results_per_view = 10 ;

    var item ;
    
 
    $.fn.tableview = function(options) {
    	var item = this ;
    	
    	var table = $(item).find('table') ;
    	    	
    	function onNewRecord() 	{
    		item.find('#create-form').formModal('show', {url: options.url + 'add/', onSuccess: function() { reload() ; }}) ;
		} ;
				
		function onEditRecord() 	{
			var id = $(this).closest('tr').data('id') ;
			item.find('#edit-form').formModal('show', {init: true, url: options.url + 'update/',  data: {id: id}, onSuccess: function() { reload() ; }}) ;
		} ;
				
		function onDeleteRecord() {
			var id = $(this).closest('tr').data('id') ;
			 $.ajax({
	    		url: options['url'] + 'delete/',
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
		

		function fillTable(data) {
			var tbody = table.find('tbody');
			tbody.empty() ;
			
			var columns = [] ;
			
			table.find('thead th').each(function(i, e) {
				var data = $(e).data('col') ;
				if ( typeof data !== 'undefined' ) 
					columns.push(data) ;
				else 
					columns.push('#buttons') ;
			}) ;
			
			for (var i = 0; i < data.rows.length; i++) {
			    var row = $('<tr/>');
			    row.attr('data-id', data.rows[i].id) ;
				for (var col = 0; col < columns.length; col++) {
					var col_name = columns[col] ;
					var cell ;
					if ( col_name === '#buttons' )
						cell = '<td align="center" style="width: 100px;"><a id="edit-button" class="btn btn-default"><em class="fa fa-pencil"></em></a><a id="delete-button" class="btn btn-danger"><em class="fa fa-trash"></em></a></td>' ;
					else cell = '<td>' + data.rows[i].data[col_name] + '</td>' ;
						
					row.append($(cell)) ;
				}
				$(tbody).append(row) ;
			}
		}
		
		function fillPager(data) {
			item.find('#pager-label').text('Page ' + data.page + ' of ' + data.total_pages) ;
		}
				
    	function reload() {

    		
    		 $.ajax({
	    		url: options['url'] + 'list/',
		   	    type: 'GET',
		   	 /*   dataType: 'json',*/
		   	    data: $.param({page: current_page, total: results_per_view}),
	    		success: function(data, textStatus, jqXHR)	{
	    			fillTable(data) ;
	    			fillPager(data) ;

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
					$(table).find('#delete-button').click(onDeleteRecord) ;
		  				
					// handle edit record
					$(table).find('#edit-button').click(onEditRecord) ;
	
	//				reload() ;
					},
				error: function(jqXHR, textStatus, errorThrown)	{
       			    // Handle errors here
       			    console.log('ERRORS: ' + textStatus);
       			    // STOP LOADING SPINNER
       			}
       			
	    	}) ;
	    	
		
		}
    
		reload() ;
					

        return this;
    };
 
}( jQuery ));
