<?php

/**
 * Inserts a tracking image.
 */
function email_analytics_insert_tracking_image($hash, &$body) {
	global $base_url;
	$query = array(
		'email_analytics_hash' => $hash,
	//	'action' => 'open',
	);
	$path = 'email_analytics';
	$link = url($path, array(
		'query' => $query,
		'absolute' => TRUE,
	));
	$body[] = theme('image', array(
		'path' => $link,
		'width' => '1px',
		'height' => '1px',
	));
}

/**
 * Insert tracking for all the links.
 */
function email_analytics_insert_link_tracking($hash, &$body) {
	global $base_url;
	$doc = new DomDocument();
	@$doc->loadHTML($body);
	$links = $doc->getElementsByTagName('a');
	foreach ($links as $link) {
		$href = $link->getAttribute('href');
		if (strpos($href, $base_url) !== FALSE) {
			$href = drupal_parse_url($href);
			$query = array_merge($href['query'], array('email_analytics_hash' => $hash));
			$link->setAttribute('href', url($href['path'], array('query' => $query)));
		}
	}

	// Convert back from html entities.
	$body = html_entity_decode($doc->saveHTML());
}

/**
 * Create a new record in email_analytics_data table.
 */
function email_analytics_write_data($eid, $action, $url = NULL, $page_title = NULL) {
	$result = db_insert('email_analytics_data')
		->fields(array(
			'eid' => $eid,
			'action' => $action,
			'url' => $url,
			'page_title' => $page_title,
			'created' => time(),
		))
		->execute();
}

/**
 * Track email opens.
 */
function email_analytics_track_email_open() {
	// If there is no hash supplied, don't do anything.
	if (empty($_GET['email_analytics_hash'])) {
		return;
	}

	$hash = $_GET['email_analytics_hash'];

	// Make sure that this hash exists in the database.
	$eid = db_select('email_analytics_info', 'i')
		->fields('i', array('eid'))
		->condition('i.email_hash', $hash)
		->execute()
		->fetchField();

	// If a valid eid is not returned, then don't do anything.
	if (!$eid) {
		return;
	}

	// A valid email was opened. Record this event in the database.
	$result = db_insert('email_analytics_data')
		->fields(array(
			'eid' => $eid,
			'action' => 'open',
			'created' => time(),
		))
		->execute();
}

