$(document).ready(function () {
	$('#generate_show').click(function () {
		var obj = this;
		var message = $('#generate_message');

		obj.disabled = true;
		message.html('Generating file...');
		$.ajax({
			url: window.location.href + '/generate',
			success: function(data) {
				message.html('File generated. New filesize: ' + data.size);
				obj.disabled = false;
			},
			error: function (r, error) {
				message.html('An error occured: ' + error);
				obj.disabled = false;
			}
		});
	});
});
