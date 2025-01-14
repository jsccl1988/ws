(function ( $ ) {
 
    var current_page = 1 ;
  	var results_per_view = 10 ;
    var total_pages ;

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
						cell = '<td align="center" style="width: 100px;"><a class="edit-button btn btn-default"><em class="fa fa-pencil"></em></a><a class="delete-button btn btn-danger"><em class="fa fa-trash"></em></a></td>' ;
					else {
                        var text = data.rows[i].data[col_name];
                        if ( typeof options.filters != 'undefined' && typeof options.filters[col_name] !== 'undefined' ) 
                            text = options.filters[col_name](data.rows[i]) ;
                        cell = '<td>' + text + '</td>' ;
                    }
						
					row.append($(cell)) ;
				}
				$(tbody).append(row) ;
			}
		}

        function make_pager_data(page, max_page) {
            var pages = {};
            
            pages['total_pages'] = max_page ;
            pages['page'] = page ;
            
            if ( max_page == 1 ) return pages ;

            var delta = 4 ;

            var min_surplus = (page <= delta) ? (delta - page + 1) : 0;
            var max_surplus = (page >= (max_page - delta)) ?
                       (page - (max_page - delta)) : 0;

            var start =  Math.max(page - delta - max_surplus, 1) ;
            var stop =   Math.min(page + delta + min_surplus, max_page) ;


            pages['first'] = {"page": 1, 'disabled': (page == 1)} ;

            if ( page > 1 )
                pages['previous'] = {"page": page-1} ;
            else
                pages['previous'] = {"disabled": true} ;


            var page_entries = [] ;
    
            for( var p = start ; p <= stop ; p++ )
            {
                if ( p == page ) page_entries.push({"active": true, "page": p, "text": p }); // no need to create a link to current page
                else {
                    page_entries.push({"page": p, "text": p }); // no need to create a link to current page
                }
            }

            pages['pages'] = page_entries ;

            if ( page < max_page )
                pages['next'] = {'page': page+1 } ;
            else
                pages['next'] = {'disabled': true} ;

            pages['last'] = {"page": max_page, 'disabled': (page == max_page)} ;

           

            return pages ;
        }
		
		function fillPager(pager) {
            var container =  $(item).find('#pager ul') ; 
            container.empty() ;
            total_pages = pager.total_pages ;

			item.find('#pager-label').text('Page ' + pager.page + ' of ' + total_pages) ;


            if ( pager.first ) {
                var button = $('<li><a href="#" data-page="' + pager.first.page + '" title="First"><i class="fa fa-backward" aria-hidden="true"></i></a></la>' ) ;
                if ( pager.first.disabled ) $(button).addClass('disabled') ;
                container.append(button) ;
            }
            if ( pager.previous ) {
                var button = $('<li><a href="#" data-page="' + pager.previous.page + '" title="Previous"><i class="fa fa-caret-left" aria-hidden="true"></i></a></la>' ) ;
                if ( pager.previous.disabled ) $(button).addClass('disabled') ;
                container.append(button) ;
            }
            for ( i in pager.pages ) {
                var page = pager.pages[i] ;
                 var button = $('<li><a href="#" data-page="' + page.page + '" title="Jump to page "' + page.page + '">' + page.text + '</a></la>' ) ;
                 if ( page.active )  $(button).addClass('active') ;  
                container.append(button) ;
            }
            if ( pager.next ) {
                var button = $('<li><a href="#" data-page="' + pager.next.page + '" title="Next"><i class="fa fa-caret-right" aria-hidden="true"></i></a></la>' ) ;
                if ( pager.next.disabled ) $(button).addClass('disabled') ;
                container.append(button) ;
            }
            if ( pager.last ) {
                var button = $('<li><a href="#" data-page="' + pager.last.page + '" title="Last"><i class="fa fa-forward" aria-hidden="true"></i></a></la>' ) ;
                if ( pager.last.disabled ) $(button).addClass('disabled') ;
                container.append(button) ;
            }
    	}
				
    	function reload() {

    		
    		 $.ajax({
	    		url: options['url'] + 'list/',
		   	    type: 'GET',
		   	 /*   dataType: 'json',*/
		   	    data: $.param({page: current_page, total: results_per_view}),
	    		success: function(data, textStatus, jqXHR)	{
	    			fillTable(data) ;
	    			fillPager(make_pager_data(data.page, data.total_pages)) ;

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
						
					if ( current_page > total_pages ) current_page = 1 ;
						
					// handle results per view change

					$(rpv_buttons).change(function(){
						results_per_view = $(this).val() ;
						reload() ;
					})
						
					// handle new record action
						
					$(item).find('#new-record').click(onNewRecord) ;

					// handle detete record		  				
		  			$(table).find('.delete-button').click(onDeleteRecord) ;

					// handle edit record
					$(table).find('.edit-button').click(onEditRecord) ;
	
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
