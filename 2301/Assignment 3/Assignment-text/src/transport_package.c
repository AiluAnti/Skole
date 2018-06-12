#include <stdlib.h>
#include <memory.h>

#include "transport_package_impl.h"

transport_package_t* transport_pkg_create(void* data, size_t size, int ack)
{
	transport_package_t* pkg;

	if (!data || !size)
		return NULL;

	pkg = (transport_package_t*)malloc(sizeof(transport_package_t));
	if (!pkg)
		return NULL;

	pkg->size = size;
	pkg->data = data;
	pkg->checksum = NULL;
	pkg->ack = ack;

	return pkg;
}

void transport_pkg_destroy(transport_package_t* tp_pkg)
{
	if (tp_pkg)
		free(tp_pkg);
}

transport_package_t* transport_pkg_copy(transport_package_t* tp_pkg)
{
	transport_package_t* cpy;

	if (!tp_pkg || !(tp_pkg->data) || !(tp_pkg->size) || !(tp_pkg->checksum) || !(tp_pkg->ack))
		return NULL;

	cpy = (transport_package_t*)malloc(sizeof(transport_package_t) + tp_pkg->size);
	if (!cpy)
		return NULL;

	cpy->size = tp_pkg->size;
	cpy->data = cpy + 1;
	cpy->checksum = tp_pkg->checksum;
	cpy->ack = tp_pkg->ack;

	memcpy(cpy->data, tp_pkg->data, tp_pkg->size);
	return cpy;
}

size_t transport_pkg_size(transport_package_t* pkg)
{
	if (!pkg)
		return 0U;
	return pkg->size;
}

void* transport_pkg_data(transport_package_t* pkg)
{
	if (!pkg)
		return NULL;
	return pkg->data;
}
