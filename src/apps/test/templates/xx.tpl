<div class="col-md-12">
<div id="page-content">
<ul>
<% 
for( uint i=0; i<10; i++ ) {
	ostringstream strm;
	strm << i % 3;
%>
	
	<li><%=strm.str()%></li>
	
<%
}
%>
</ul>
</div>
<input name="image<%=key%>" type="file" id="upload" class="hidden" onchange="">
</div>



